#!/usr/bin/env python
import sys

if len(sys.argv) != 3:
  print "Usage: " + sys.argv[0] + " <truth file> <results file>"
  exit()

try:
  truthFile = open(sys.argv[1], "r")
except IOError:
  print "Invalid truth file: " + sys.argv[1]
  exit()

truth = {}
for line in truthFile.readlines():
  elements = line.split()
  if len(elements) != 2:
    print "Invalid truth data: " + line
    exit()
  truth[elements[0]] = elements[1]
truthFile.close()

totalImages = 0
numCorrect = 0
numFalseNegatives = 0
numErrors = 0
errorImages = []
nonDetectImages = []

try:
  resultsFile = open(sys.argv[2], "r")
except IOError:
  print "Invalid results file: " + sys.argv[2]
  exit()

# Skip the header line
results = resultsFile.readlines()[1:]
resultsFile.close()

for line in results:
  elements = line.split()
  if len(elements) != 2:
    print "Skipping invalid results data: " + line
    continue
  if truth[elements[0]] == elements[1]:
    numCorrect = numCorrect + 1
  elif elements[1] == "None":
    numFalseNegatives = numFalseNegatives + 1
    nonDetectImages = nonDetectImages + [elements[0]]
  else:
    numErrors = numErrors + 1
    errorImages = errorImages + [elements[0]]
  totalImages = totalImages + 1

if totalImages != 0:
  correctPercent = float(numCorrect) / totalImages * 100
  errorPercent = float(numErrors) / totalImages * 100
  nonDetectPercent = float(numFalseNegatives) / totalImages * 100
else:
  correctPercent = 0
  errorPercent = 0
  nonDetectPercent = 0

correctStr = "Correct:    " + str(correctPercent) + "%\t(" + str(numCorrect) + " / " + str(totalImages) + ")"
errorStr = "Error:      " + str(errorPercent) + "%\t(" + str(numErrors) + " / " + str(totalImages) + ")"
undetectedStr = "Undetected: " + str(nonDetectPercent) + "%\t(" + str(numFalseNegatives) + " / " + str(totalImages) + ")"

print correctStr
print errorStr
print undetectedStr

scoreFilename = sys.argv[2] + ".score"
try:
  scoreFile = open(scoreFilename, "w")
except IOError:
  print "Couldn't open score file: " + scoreFilename
  exit()

scoreFile.write(correctStr + "\n")
scoreFile.write(errorStr + "\n")
scoreFile.write(undetectedStr + "\n")
scoreFile.write("\n")
scoreFile.write("Error Images:\n")
for image in errorImages:
  scoreFile.write(image + "\n")
scoreFile.write("\n")
scoreFile.write("Undetected Images:\n")
for image in nonDetectImages:
  scoreFile.write(image + "\n")
scoreFile.close()