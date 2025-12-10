#include "storageservice.h"

#include <algorithm>
#include <cstdio>

#ifdef ESP_PLATFORM
#include <esp_random.h>
#else
#include <cstdlib>
#include <ctime>
#endif

StorageService::StorageService() {
#ifndef ESP_PLATFORM
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        seeded = true;
    }
#endif
}

StorageService::ExerciseId StorageService::generateId() {
    ExerciseId id{};
    size_t offset = 0;
    while (offset < id.size()) {
#ifdef ESP_PLATFORM
        uint32_t randomValue = esp_random();
#else
        uint32_t randomValue = static_cast<uint32_t>(std::rand());
#endif
        for (int byteIndex = 0; byteIndex < 4 && offset < id.size(); ++byteIndex) {
            id[offset++] = static_cast<uint8_t>((randomValue >> (byteIndex * 8)) & 0xFF);
        }
    }
    return id;
}

bool StorageService::idExists(const ExerciseId& id) const {
    return std::any_of(exercises_.begin(), exercises_.end(), [&](const ExerciseRecord& record) {
        return record.id == id;
    });
}

StorageService::ExerciseId StorageService::addExercise(const Exercise& exercise) {
    ExerciseId id = generateId();
    while (idExists(id)) {
        id = generateId();
    }

    exercises_.push_back({id, exercise});
    return id;
}

bool StorageService::updateExercise(const ExerciseId& id, const Exercise& exercise) {
    auto it = std::find_if(exercises_.begin(), exercises_.end(), [&](const ExerciseRecord& record) {
        return record.id == id;
    });
    if (it == exercises_.end()) {
        return false;
    }
    it->exercise = exercise;
    return true;
}

bool StorageService::removeExercise(const ExerciseId& id) {
    auto it = std::find_if(exercises_.begin(), exercises_.end(), [&](const ExerciseRecord& record) {
        return record.id == id;
    });
    if (it == exercises_.end()) {
        return false;
    }
    exercises_.erase(it);
    return true;
}

void StorageService::clear() {
    exercises_.clear();
}

Exercise* StorageService::findExercise(const ExerciseId& id) {
    auto it = std::find_if(exercises_.begin(), exercises_.end(), [&](const ExerciseRecord& record) {
        return record.id == id;
    });
    return it != exercises_.end() ? &it->exercise : nullptr;
}

const Exercise* StorageService::findExercise(const ExerciseId& id) const {
    auto it = std::find_if(exercises_.begin(), exercises_.end(), [&](const ExerciseRecord& record) {
        return record.id == id;
    });
    return it != exercises_.end() ? &it->exercise : nullptr;
}

StorageService::ExerciseRecord* StorageService::findRecord(const ExerciseId& id) {
    auto it = std::find_if(exercises_.begin(), exercises_.end(), [&](const ExerciseRecord& record) {
        return record.id == id;
    });
    return it != exercises_.end() ? &(*it) : nullptr;
}

const StorageService::ExerciseRecord* StorageService::findRecord(const ExerciseId& id) const {
    auto it = std::find_if(exercises_.begin(), exercises_.end(), [&](const ExerciseRecord& record) {
        return record.id == id;
    });
    return it != exercises_.end() ? &(*it) : nullptr;
}

StorageService::ExerciseId StorageService::fromHex(const String& hex) {
    ExerciseId id{};
    if (hex.length() != id.size() * 2) {
        return id;
    }

    auto nibble = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return -1;
    };

    for (size_t i = 0; i < id.size(); ++i) {
        int highVal = nibble(hex[i * 2]);
        int lowVal = nibble(hex[i * 2 + 1]);
        if (highVal < 0 || lowVal < 0) {
            id.fill(0);
            return id;
        }
        id[i] = static_cast<uint8_t>((highVal << 4) | lowVal);
    }
    return id;
}

String StorageService::toHex(const ExerciseId& id) {
    char buffer[id.size() * 2 + 1];
    for (size_t i = 0; i < id.size(); ++i) {
        std::snprintf(&buffer[i * 2], 3, "%02X", id[i]);
    }
    buffer[id.size() * 2] = '\0';
    return String(buffer);
}
