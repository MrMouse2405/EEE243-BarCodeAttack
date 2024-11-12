#include "Parser.h"
#include "Arduino.h"
#include "code39.h"

/**
 * Parser
 *
 * Defines the KNNParser class for parsing and classifying barcode values
 * using the K-Nearest Neighbor algorithm. Supports training with calibration
 * data and batch predictions as code39 characters.
 *
 * Date: 2024-11-11
 *
 */

using namespace Parser;

/*
* Trains the KNN model using 9 labeled barcode width values from a calibration batch.
*/
void KNNParser::train(const Lab4::Buffer<Lab4::Bar,WIDTH_CHARACTER_SIZE> *calibrationBatch) {
    for (int i = 0; i < WIDTH_CHARACTER_SIZE; i++) {
        this->trainingData[i].type = calibrationBatch->buffer[i].type;
        this->trainingData[i].time = calibrationBatch->buffer[i].time;
        this->points[i].bar = &trainingData[i];
    }
}

/*
 * Predicts whether a given barcode width is Narrow or Wide based on calibration data.
 */
Lab4::BarType KNNParser::getBarType(const Lab4::Bar *bar) {
    return KNearestClassifier(bar, 3, this->points);
}

/*
 * Decodes a Code39 character from a sequence of Narrow/Wide values.
 * Returns an Option<char>; empty if the sequence does not conform to Code39 specifications.
 */
Lab4::Option<char> KNNParser::lex(const Lab4::Buffer<Lab4::BarType,WIDTH_CHARACTER_SIZE> &code) {
    for (auto i: code39) {
        // Set string length in code39.h is 9 excluding the first character
        if (strncmp(reinterpret_cast<const char *>(code.buffer), i + 1, WIDTH_CHARACTER_SIZE) == 0) {
            return Lab4::Option<char>(i[0]);
        }
    }

    return {};
}


/*
 * Classifies a bar using the k-nearest neighbors algorithm.
 * Assumes two groups: Narrow and Wide. Returns Narrow if
 * the majority of the k nearest neighbors belong to Narrow;
 * otherwise, returns Wide.
 */
Lab4::BarType KNNParser::KNearestClassifier(const Lab4::Bar *bar, const int k, KNNPoint points[WIDTH_CHARACTER_SIZE]) {
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

/*
 * Sorts an array of KNNPoint using the quicksort algorithm.
 * This function organizes the points based on their distances
 * to facilitate efficient neighbor classification.
 */
void KNNParser::quickSort(KNNPoint arr[], int low, int high) {
    if (low < high) {
        // call partition function to find Partition Index
        const int partitionIndex = partition(arr, low, high);

        // Recursively call quickSort() for left and right
        // half based on Partition Index
        quickSort(arr, low, partitionIndex - 1);
        quickSort(arr, partitionIndex + 1, high);
    }
}

/*
 * Partitions the array of KNNPoint elements for quicksort.
 * Rearranges the elements so that those less than a pivot
 * come before it, and those greater come after.
 */
int KNNParser::partition(KNNPoint arr[], int low, int high) {
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

/*
 * Swaps two elements of type T.
 * This function is used in the quicksort algorithm to exchange
 * positions of elements in the array.
 */
template<typename T>
void KNNParser::swap(T *a, T *b) {
    T temp = *a;
    *a = *b;
    *b = temp;
}
