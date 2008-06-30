#include "TimingAnalyzer.h"
#include <stdio.h>
#include <sys/time.h>
#include <vector>
using namespace std;

vector< vector<int> > gTimeStats;
vector< struct timeval > gStartTimes;

void TimingAnalyzer_Start(int id)
{
  if ( id >= (int)gTimeStats.size() )
  {
    gTimeStats.resize(id + 1);
    gStartTimes.resize(id + 1);
  }
  gettimeofday(&gStartTimes[id], NULL);
}

void TimingAnalyzer_Stop(int id)
{
  timeval stopTime;
  int microSeconds;

  if ( id >= (int)gTimeStats.size() )
    return;

  gettimeofday(&stopTime, NULL);
  microSeconds = (stopTime.tv_sec - gStartTimes[id].tv_sec) * 1000000 +
                 stopTime.tv_usec - gStartTimes[id].tv_usec;
  gTimeStats[id].push_back(microSeconds);
}

int TimingAnalyzer_Min(int id)
{
  int i, len, minTime;

  if ( (id >= (int)gTimeStats.size()) || gTimeStats[id].empty() )
    return -1;

  minTime = gTimeStats[id][0];
  len = gTimeStats[id].size();
  for (i = 1; i < len; i++)
    if ( gTimeStats[id][i] < minTime )
      minTime = gTimeStats[id][i];

  return minTime;
}

int TimingAnalyzer_Mean(int id)
{
  int i, len, sum;

  if ( (id >= (int)gTimeStats.size()) || gTimeStats[id].empty() )
    return -1;

  sum = 0;
  len = gTimeStats[id].size();
  for (i = 0; i < len; i++)
  {
    sum += gTimeStats[id][i];
    if ( sum < 0 )
    {
      fprintf(stderr, "TimingAnalyzer_Mean - Rollover error\n");
      return -1;
    }
  }

  return sum / len;
}

int TimingAnalyzer_Max(int id)
{
  int i, len, maxTime;

  if ( (id >= (int)gTimeStats.size()) || gTimeStats[id].empty() )
    return -1;

  maxTime = gTimeStats[id][0];
  len = gTimeStats[id].size();
  for (i = 1; i < len; i++)
    if ( gTimeStats[id][i] > maxTime )
      maxTime = gTimeStats[id][i];

  return maxTime;
}
