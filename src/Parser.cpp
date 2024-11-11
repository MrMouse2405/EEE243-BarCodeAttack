#include "Parser.h"
#include "Arduino.h"
#include "code39.h"
using namespace Parser;

template<typename T>
void swap(T* a, T* b) {
    T temp = *a;
    *a = *b;
    *b = temp;
}


int partition(KNNPoint arr[], int low, int high) {
    // Initialize pivot to be the first element
    int p = arr[low].distance;
    int i = low;
    int j = high;

    while (i < j) {
        // Find the first element greater than
        // the pivot (from starting)
        while (arr[i].distance <= p && i <= high - 1) {
            i++;
        }

        // Find the first element smaller than
        // the pivot (from last)
        while (arr[j].distance > p && j >= low + 1) {
            j--;
        }
        if (i < j) {
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[low], &arr[j]);
    return j;
}


void quickSort(KNNPoint arr[], int low, int high) {
    if (low < high) {
        // call partition function to find Partition Index
        const int partitionIndex = partition(arr, low, high);

        // Recursively call quickSort() for left and right
        // half based on Partition Index
        quickSort(arr, low, partitionIndex - 1);
        quickSort(arr, partitionIndex + 1, high);
    }
}

// This function finds classification of bar using
// k nearest neighbour algorithm. It assumes only two
// groups and returns Narrow if bar belongs to group Narrow, else
// Wide.
inline Lab4::BarType KNearestClassifier(const Lab4::Bar *bar, const int k, KNNPoint points[WIDTH_CHARACTER_SIZE]) {
    // calculate euclidian distance
    for (int i = 0; i < WIDTH_CHARACTER_SIZE; i++) {
        points[i].distance = abs(
            static_cast<int64_t>(bar->time) - static_cast<int64_t>(points[i].bar->time)
        );
    }

    quickSort(points, 0,WIDTH_CHARACTER_SIZE - 1);

    // Now consider the first k elements and only
    // two groups
    int NarrowFrequency = 0; // Frequency of group 0
    int WideFrequency = 0; // Frequency of group 1
    for (int i = 0; i < k; i++) {
        if (points[i].bar->type == Lab4::BarType::Narrow) {
            NarrowFrequency++;
        } else {
            WideFrequency++;
        }
    }

    return NarrowFrequency > WideFrequency ? Lab4::BarType::Narrow : Lab4::BarType::Wide;
}


Lab4::BarType KNNParser::getBarType(const Lab4::Bar *bar) {
    return KNearestClassifier(bar, 3, this->points);
}


void KNNParser::train(const Lab4::Buffer<Lab4::Bar,WIDTH_CHARACTER_SIZE> *calibrationBatch) {
    for (int i = 0; i < WIDTH_CHARACTER_SIZE; i++) {
        this->trainingData[i].type = calibrationBatch->buffer[i].type;
        this->trainingData[i].time = calibrationBatch->buffer[i].time;
        this->points[i].bar = &trainingData[i];
    }
}

Lab4::Option<char> KNNParser::lex(const Lab4::Buffer<Lab4::BarType,WIDTH_CHARACTER_SIZE>& code) {
    for (auto i: code39) {
        // Set string length in code39.h is 9 excluding the first character
        if (strncmp(reinterpret_cast<const char *>(code.buffer), i + 1, WIDTH_CHARACTER_SIZE) == 0) {
            return Lab4::Option<char>(i[0]);
        }
    }

    return {};
}
