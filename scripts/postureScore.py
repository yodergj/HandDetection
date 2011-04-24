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

closeClasses = { "closed" : [ "four", "vulcan" ],
                 "fist" : [ "pinky", "point", "thumb" ],
                 "four" : [ "closed", "open", "three" ],
                 "hangloose" : [ "thumb", "pinky" ],
                 "open" : [ "four", "vulcan" ],
                 "pinky" : [ "fist", "hangloose" ],
                 "point" : [ "fist", "two" ],
                 "three" : [ "four", "two" ],
                 "thumb" : [ "fist", "hangloose" ],
                 "two" : [ "point", "three" ],
                 "vulcan" : [ "closed", "open" ] }

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
numCorrectDetect = 0
numCorrectNoDetect = 0
numFalseNegatives = 0
numErrors = 0
numFalsePositives = 0
numMisclassified = 0
numCloseErrors = 0
numMulti = 0
numMultiWithCorrect = 0
numMultiNoCorrect = 0
misclassifiedImages = []
falsePositiveImages = []
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
  try:
    truth[elements[0]]
  except KeyError:
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
    if elements[1] == "None":
      numCorrectNoDetect = numCorrectNoDetect + 1
    else:
      numCorrectDetect = numCorrectDetect + 1
  elif elements[1] == "None":
    numFalseNegatives = numFalseNegatives + 1
    nonDetectImages = nonDetectImages + [elements[0]]
  else:
    numErrors = numErrors + 1
    if truth[elements[0]] == "None":
      numFalsePositives = numFalsePositives + 1
      falsePositiveImages = falsePositiveImages + [elements[0]]
    else:
      if elements[1] in closeClasses[ truth[elements[0]] ]:
        numCloseErrors = numCloseErrors + 1
      numMisclassified = numMisclassified + 1
      misclassifiedImages = misclassifiedImages + [elements[0]]
  totalImages = totalImages + 1

if totalImages != 0:
  correctPercent = float(numCorrect) / totalImages * 100
  correctNoDetectPercent = float(numCorrectNoDetect) / totalImages * 100
  correctDetectPercent = float(numCorrectDetect) / totalImages * 100
  errorPercent = float(numErrors) / totalImages * 100
  misclassifiedPercent = float(numMisclassified) / totalImages * 100
  closePercent = float(numCloseErrors) / totalImages * 100
  falsePositivePercent = float(numFalsePositives) / totalImages * 100
  nonDetectPercent = float(numFalseNegatives) / totalImages * 100
  multiPercent = float(numMulti) / totalImages * 100
  multiCorrectPercent = float(numMultiWithCorrect) / totalImages * 100
  multiErrorPercent = float(numMultiNoCorrect) / totalImages * 100
else:
  correctPercent = 0
  correctNoDetectPercent = 0
  correctDetectPercent = 0
  errorPercent = 0
  misclassifiedPercent = 0
  closePercent = 0
  falsePositivePercent = 0
  nonDetectPercent = 0
  multiPercent = 0
  multiCorrectPercent = 0
  multiErrorPercent = 0

correctStr = "Correct:    " + str(correctPercent) + "%\t(" + str(numCorrect) + " / " + str(totalImages) + ")"
correctPosStr = "  Positive: " + str(correctDetectPercent) + "%\t(" + str(numCorrectDetect) + " / " + str(totalImages) + ")"
correctNegStr = "  Negative: " + str(correctNoDetectPercent) + "%\t(" + str(numCorrectNoDetect) + " / " + str(totalImages) + ")"
errorStr = "Error:      " + str(errorPercent) + "%\t(" + str(numErrors) + " / " + str(totalImages) + ")"
misclassifiedStr = "  Misclass: " + str(misclassifiedPercent) + "%\t(" + str(numMisclassified) + " / " + str(totalImages) + ")"
closeStr = "    Close:  " + str(closePercent) + "%\t(" + str(numCloseErrors) + " / " + str(totalImages) + ")"
falsePosStr = "  False +:  " + str(falsePositivePercent) + "%\t(" + str(numFalsePositives) + " / " + str(totalImages) + ")"
undetectedStr = "Undetected: " + str(nonDetectPercent) + "%\t(" + str(numFalseNegatives) + " / " + str(totalImages) + ")"
multiStr = "Multiples:  " + str(multiPercent) + "%\t(" + str(numMulti) + " / " + str(totalImages) + ")"
multiCorrectStr = "  Correct:  " + str(multiCorrectPercent) + "%\t(" + str(numMultiWithCorrect) + " / " + str(totalImages) + ")"
multiErrorStr = "  Error:    " + str(multiErrorPercent) + "%\t(" + str(numMultiNoCorrect) + " / " + str(totalImages) + ")"

print correctStr
print correctPosStr
print correctNegStr
print errorStr
print misclassifiedStr
print closeStr
print falsePosStr
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
scoreFile.write(correctPosStr + "\n")
scoreFile.write(correctNegStr + "\n")
scoreFile.write(errorStr + "\n")
scoreFile.write(misclassifiedStr + "\n")
scoreFile.write(closeStr + "\n")
scoreFile.write(falsePosStr + "\n")
scoreFile.write(undetectedStr + "\n")
scoreFile.write(multiStr + "\n")
scoreFile.write(multiCorrectStr + "\n")
scoreFile.write(multiErrorStr + "\n")
scoreFile.write("\n")
scoreFile.write("Misclassified Images:\n")
for image in misclassifiedImages:
  scoreFile.write(image + "\n")
scoreFile.write("\n")
scoreFile.write("False Positive Images:\n")
for image in falsePositiveImages:
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