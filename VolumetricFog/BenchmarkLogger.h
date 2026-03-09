#pragma once
#include <fstream>
#include <vector>
#include <string>
#include <iostream>

class BenchmarkLogger {
    struct DataPoint {
        float timeFroxel;
        float timeRaymarch;
        int frameID;
    };
    std::vector<DataPoint> logs;
    bool isRecording = false;

public:
    void StartRecording() {
        logs.clear();
        isRecording = true;
        std::cout << "Benchmark STARTED..." << std::endl;
    }

    void StopRecording(const std::string& filename) {
        isRecording = false;
        std::ofstream file(filename);

        // CSV Header
        file << "Frame,Froxel_ms,Raymarch_ms\n";

        for (const auto& p : logs) {
            file << p.frameID << "," << p.timeFroxel << "," << p.timeRaymarch << "\n";
        }
        file.close();
        std::cout << "Benchmark SAVED to " << filename << std::endl;
    }

    void LogFrame(int frameID, float froxelTime, float raymarchTime) {
        if (!isRecording) return;
        logs.push_back({ froxelTime, raymarchTime, frameID });
    }

    bool IsRecording() const { return isRecording; }
};
