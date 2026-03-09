#pragma once
#include <d3d11.h>
#include <vector>
#include <string>
#include <map>

static constexpr int QUERY_LATENCY_FRAMES = 3;

struct ProfilerFrame {
	ID3D11Query* disjointQuery;
	ID3D11Query* startQuery;
	ID3D11Query* endQuery;
	bool queryStarted = false;
	bool queryEnded = false;
};

class GPUProfiler {
	ID3D11Device* device;
	ID3D11DeviceContext* context;

	ProfilerFrame frames[QUERY_LATENCY_FRAMES];
	int currentFrameIndex = 0;

	double lastMeasuredTimeMS = 0.0;

public:
	void Initialize(ID3D11Device* devicePtr, ID3D11DeviceContext* contextPtr) {
		device = devicePtr;
		context = contextPtr;

		D3D11_QUERY_DESC desc = {};
		for (int i = 0; i < QUERY_LATENCY_FRAMES; i++) {
			desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
			device->CreateQuery(&desc, &frames[i].disjointQuery);

			desc.Query = D3D11_QUERY_TIMESTAMP;
			device->CreateQuery(&desc, &frames[i].startQuery);
			device->CreateQuery(&desc, &frames[i].endQuery);
		}
	}

	void BeginProfile() {
		ProfilerFrame& frame = frames[currentFrameIndex];

		context->Begin(frame.disjointQuery); // Start tracking

		context->End(frame.startQuery); // Mark start time
		frame.queryStarted = true;
	}

	void EndProfile() {
		ProfilerFrame& frame = frames[currentFrameIndex];
		if (!frame.queryStarted) return;

		context->End(frame.endQuery); // Mark end time

		context->End(frame.disjointQuery);
		frame.queryEnded = true;
	}

	void ResolveData() {
		currentFrameIndex = (currentFrameIndex + 1) % QUERY_LATENCY_FRAMES;

		ProfilerFrame& oldFrame = frames[currentFrameIndex];
		if (oldFrame.queryEnded) {
			D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
			
			if (context->GetData(oldFrame.disjointQuery, &disjointData, sizeof(disjointData), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK) {
				if (!disjointData.Disjoint) {
					UINT64 start = 0, end = 0;
					context->GetData(oldFrame.startQuery, &start, sizeof(start), 0);
					context->GetData(oldFrame.endQuery, &end, sizeof(end), 0);

					double delta = double(end - start);
					double freq = double(disjointData.Frequency);
					lastMeasuredTimeMS = (delta / freq) * 1000.0;
				}
			}
			
			oldFrame.queryEnded = false;
			oldFrame.queryStarted = false;
		}

	}

	double GetTimeMS() const { return lastMeasuredTimeMS; }
};