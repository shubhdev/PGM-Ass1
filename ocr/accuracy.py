import sys
actual = sys.argv[1]
prediction = sys.argv[2]

f = open(actual, 'r')
actual = f.read().split()
f = open(prediction, 'r')
prediction = f.read().split('\n')
prediction = [x.split() for x in prediction]
# print(actual)
correct = 0.0
for i in xrange(0,len(actual)):
	if(actual[i] == prediction[i][2]): correct += 1

print(correct,correct/len(actual))