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
numMulti = 0
numMultiWithCorrect = 0
numMultiNoCorrect = 0
errorImages = []
nonDetectImages = []
multiCorrectImages = []
multiErrorImages = []

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
  if len(elements) < 2:
    print "Skipping invalid results data: " + line
    continue
  if len(elements) > 2:
    numMulti = numMulti + 1
    correctFound = 0
    for multiResult in elements[1:]:
      if multiResult == truth[elements[0]]:
        correctFound = 1
    if correctFound:
      numMultiWithCorrect = numMultiWithCorrect + 1
      multiCorrectImages = multiCorrectImages + [elements[0]]
    else:
      numMultiNoCorrect = numMultiNoCorrect + 1
      multiErrorImages = multiErrorImages + [elements[0]]
  elif truth[elements[0]] == elements[1]:
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
  multiPercent = float(numMulti) / totalImages * 100
  multiCorrectPercent = float(numMultiWithCorrect) / totalImages * 100
  multiErrorPercent = float(numMultiNoCorrect) / totalImages * 100
else:
  correctPercent = 0
  errorPercent = 0
  nonDetectPercent = 0
  multiPercent = 0
  multiCorrectPercent = 0
  multiErrorPercent = 0

correctStr = "Correct:    " + str(correctPercent) + "%\t(" + str(numCorrect) + " / " + str(totalImages) + ")"
errorStr = "Error:      " + str(errorPercent) + "%\t(" + str(numErrors) + " / " + str(totalImages) + ")"
undetectedStr = "Undetected: " + str(nonDetectPercent) + "%\t(" + str(numFalseNegatives) + " / " + str(totalImages) + ")"
multiStr = "Multiples:  " + str(multiPercent) + "%\t(" + str(numMulti) + " / " + str(totalImages) + ")"
multiCorrectStr = "  Correct:  " + str(multiCorrectPercent) + "%\t(" + str(numMultiWithCorrect) + " / " + str(totalImages) + ")"
multiErrorStr = "  Error:    " + str(multiErrorPercent) + "%\t(" + str(numMultiNoCorrect) + " / " + str(totalImages) + ")"

print correctStr
print errorStr
print undetectedStr
print multiStr
print multiCorrectStr
print multiErrorStr

scoreFilename = sys.argv[2] + ".score"
try:
  scoreFile = open(scoreFilename, "w")
except IOError:
  print "Couldn't open score file: " + scoreFilename
  exit()

scoreFile.write(correctStr + "\n")
scoreFile.write(errorStr + "\n")
scoreFile.write(undetectedStr + "\n")
scoreFile.write(multiStr + "\n")
scoreFile.write(multiCorrectStr + "\n")
scoreFile.write(multiErrorStr + "\n")
scoreFile.write("\n")
scoreFile.write("Error Images:\n")
for image in errorImages:
  scoreFile.write(image + "\n")
scoreFile.write("\n")
scoreFile.write("Undetected Images:\n")
for image in nonDetectImages:
  scoreFile.write(image + "\n")
scoreFile.write("\n")
scoreFile.write("Multi-Detect Correct Images:\n")
for image in multiCorrectImages:
  scoreFile.write(image + "\n")
scoreFile.write("\n")
scoreFile.write("Multi-Detect Error Images:\n")
for image in multiErrorImages:
  scoreFile.write(image + "\n")
scoreFile.close()