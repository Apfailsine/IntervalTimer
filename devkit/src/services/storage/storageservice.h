#pragma once

#include <Arduino.h>
#include <array>
#include <vector>
#include "models/datastructures.h"

class StorageService {
public:
    using ExerciseId = std::array<uint8_t, 16>;

    struct ExerciseRecord {
        ExerciseId id;
        Exercise exercise;
    };

    StorageService();

    ExerciseId addExercise(const Exercise& exercise);
    bool updateExercise(const ExerciseId& id, const Exercise& exercise);
    bool removeExercise(const ExerciseId& id);
    void clear();

    Exercise* findExercise(const ExerciseId& id);
    const Exercise* findExercise(const ExerciseId& id) const;

    ExerciseRecord* findRecord(const ExerciseId& id);
    const ExerciseRecord* findRecord(const ExerciseId& id) const;

    static ExerciseId fromHex(const String& hex);
    static String toHex(const ExerciseId& id);

    const std::vector<ExerciseRecord>& exercises() const { return exercises_; }

private:
    ExerciseId generateId();
    bool idExists(const ExerciseId& id) const;

    std::vector<ExerciseRecord> exercises_;
};
