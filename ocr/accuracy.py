import sys
actual = sys.argv[1]
prediction = sys.argv[2]

f = open(actual, 'r')
actual = f.read().split()
f = open(prediction, 'r')
prediction = f.read().split('\n')
prediction = [x.split() for x in prediction]
# print(actual)
correct = 0
for i in xrange(0,len(actual)):
	print(actual[i],prediction[i][2],actual[i]==prediction[i][2])

print(correct,len(actual))