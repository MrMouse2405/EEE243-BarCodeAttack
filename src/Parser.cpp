#include "Parser.h"
#include "Arduino.h"
using namespace Parser;

static inline int getEuclidianDistance1D(int p, int q);
static inline void swap(struct KNNPoint* a, struct KNNPoint* b);
static inline int partition(struct KNNPoint arr[], int low, int high);
void quickSort(struct KNNPoint arr[], int low, int high);
static inline Lab4::BarType classifyPointAt(Lab4::Bar* bar,int k,struct KNNPoint trainingData[WIDTH_CHARACTER_SIZE]);

Lab4::BarType KNNParser::getBarType(Lab4::Bar* bar) {
    return classifyPointAt(bar,3,this->trainingData);
}

void KNNParser::train(Lab4::Batch* calibrationBatch) {
    Lab4::Bar * calibrationData = calibrationBatch->getBars();
    for (int i = 0; i < WIDTH_CHARACTER_SIZE; i++) {
       this->trainingData->klass = calibrationData[i].type;
       this->trainingData->value = calibrationData[i].time;
    }
}


// This function finds classification of bar using
// k nearest neighbour algorithm. It assumes only two
// groups and returns Narrow if bar belongs to group Narrow, else
// Wide.
static inline Lab4::BarType classifyPointAt(Lab4::Bar* bar,int k,struct KNNPoint trainingData[WIDTH_CHARACTER_SIZE]) {
    
    for (int i = 0; i < WIDTH_CHARACTER_SIZE; i++) {
        trainingData->distance = getEuclidianDistance1D(trainingData->value,bar->time);
    }

    quickSort(trainingData,0,WIDTH_CHARACTER_SIZE-1);

    // Now consider the first k elements and only
    // two groups
    int NarrowFrequency = 0;     // Frequency of group 0
    int WideFrequency   = 0;     // Frequency of group 1
    for (int i = 0; i < k; i++)
    {
        if (trainingData[i].klass == Lab4::BarType::Narrow)
            NarrowFrequency++;
        else if (trainingData[i].klass == Lab4::BarType::Wide)
            WideFrequency++;
    }

    return (NarrowFrequency > WideFrequency ? Lab4::BarType::Narrow : Lab4::BarType::Wide);
}


static inline int getEuclidianDistance1D(int p, int q) {
    return abs(p - q);
}

void quickSort(struct KNNPoint arr[], int low, int high) {
    if (low < high) {

        // call partition function to find Partition Index
        int pi = partition(arr, low, high);

        // Recursively call quickSort() for left and right
        // half based on Partition Index
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}


static inline void swap(struct KNNPoint* a, struct KNNPoint* b) {
    struct KNNPoint temp = *a;
    *a = *b;
    *b = temp;
}

static inline int partition(struct KNNPoint arr[], int low, int high) {

    // Initialize pivot to be the first element
    struct KNNPoint p = arr[low];
    int i = low;
    int j = high;

    while (i < j) {

        // Find the first element greater than
        // the pivot (from starting)
        while (arr[i].distance <= p.distance && i <= high - 1) {
            i++;
        }

        // Find the first element smaller than
        // the pivot (from last)
        while (arr[j].distance > p.distance && j >= low + 1) {
            j--;
        }
        if (i < j) {
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[low], &arr[j]);
    return j;
}
