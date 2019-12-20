#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sndfile.h>

//Compile this code using:
//gcc LowPassFilterWithSndFile.c -o LowPassFilterWithSndFile -lsndfile

#define MAX_DATA_SIZE 1024 * 1024 // 1KB = 1024, 1MB = 1024KB
#define kCutoffFreq 1000.0        //Cutoff Frequency
#define kCutoffFreqLow 100.0      //Cutoff Frequency
#define kQFactor 1.0 / sqrt(2.0)  // tightness of the filter or bandwidth

double input[MAX_DATA_SIZE] = {0}; // buffer for input
double output[MAX_DATA_SIZE] = {0}; // buffer for output
int count = 0;

void LowPassFilter(double cutoffFreq, double Q, double a[], double b[]);

int main(void)
{
  SNDFILE *input_sf;
  SNDFILE *output_sf;
  SF_INFO input_sfinfo;
  SF_INFO output_sfinfo;
  double cutoffFreq, cutoffFreqLow, Q, a[3], b[3];
  int feedback = 2, delay = 2;

  input_sfinfo.format = 0;

  input_sf = sf_open("EggShaker.wav", SFM_READ, &input_sfinfo);
  if (input_sf == NULL)
  {
    printf("Failed to open file for reading\n");
    return 1;
  }
  count = sf_read_double(input_sf, input, MAX_DATA_SIZE);

  cutoffFreq = kCutoffFreq / input_sfinfo.samplerate;
  cutoffFreqLow = kCutoffFreqLow / input_sfinfo.samplerate;
  LowPassFilter(cutoffFreq, kQFactor, a, b); // Create LPF

  for (int i = 0; i < count; i++) // Filtering process
  {
    for (int j = 0; j <= delay; j++)
    {
      if (i - j >= 0)
      {
        output[i] += b[j] * input[i - j];
      }
    }
    for (int j = 1; j <= feedback; j++)
    {
      if (i - j >= 0)
      {
        output[i] += -a[j] * output[i - j];
      }
    }
  }

  //We initialize the output with same input values
  output_sfinfo.samplerate = input_sfinfo.samplerate;
  output_sfinfo.channels = input_sfinfo.channels;
  output_sfinfo.format = input_sfinfo.format;
  output_sfinfo.frames = input_sfinfo.frames;

  output_sf = sf_open("EggShakerFiltered.wav", SFM_WRITE, &output_sfinfo);
  if (output_sf == NULL)
  {
    printf("Failed to open file for writing\n");
    return 1;
  }
  sf_write_double(output_sf, output, count);

  sf_close(input_sf);
  sf_close(output_sf);
}

void LowPassFilter(double cutoffFreq, double Q, double a[], double b[])
{
  cutoffFreq = tan(M_PI * cutoffFreq) / (2.0 * M_PI);

  a[0] = 1.0 + 2.0 * M_PI * cutoffFreq / Q + 4.0 * M_PI * M_PI * cutoffFreq * cutoffFreq;
  a[1] = (8.0 * M_PI * M_PI * cutoffFreq * cutoffFreq - 2.0) / a[0];
  a[2] = (1.0 - 2.0 * M_PI * cutoffFreq / Q + 4.0 * M_PI * M_PI * cutoffFreq * cutoffFreq) / a[0];
  b[0] = 4.0 * M_PI * M_PI * cutoffFreq * cutoffFreq / a[0]; // coefficient for current input
  b[1] = 8.0 * M_PI * M_PI * cutoffFreq * cutoffFreq / a[0]; // " " for previous
  b[2] = 4.0 * M_PI * M_PI * cutoffFreq * cutoffFreq / a[0]; // " " for 2 samples before

  a[0] = 1.0;
}