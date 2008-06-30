#ifndef _TIMING_ANALYZER
#define _TIMING_ANALYZER

void TimingAnalyzer_Start(int id);
void TimingAnalyzer_Stop(int id);
int TimingAnalyzer_Min(int id);
int TimingAnalyzer_Mean(int id);
int TimingAnalyzer_Max(int id);

#endif
