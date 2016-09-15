import sys

def mismatch(s,t):
		assert(len(s) == len(t))
		res = 0
		for i in xrange(0,len(s)):
				if (s[i] != t[i]) :
						res += 1
		return res

actual = sys.argv[1]
prediction = sys.argv[2]

f = open(actual, 'r')
actual = f.read().split()
f = open(prediction, 'r')
prediction = f.read().split('\n')
prediction = [x.split() for x in prediction]
# print(actual)
char_acc = [0.0,0.0,0.0]
word_acc = [0.0,0.0,0.0]

ex1 = open('ex1.txt', 'w')
ex2 = open('ex2.txt','w')
for i in xrange(0,len(actual)):
		word = actual[i]
		for j in xrange(0,3):
				pred = prediction[i][j]
				if(pred == word): word_acc[j] += 1
				for k in xrange(0,len(word)):
						if(word[k] == pred[k]): char_acc[j] += 1
		if(word != prediction[i][0] and word == prediction[i][1]):
				ex1.write('{} {}\n'.format(i+1,word))
		if(mismatch(word,prediction[i][0]) > mismatch(word,prediction[i][1])
			 and mismatch(word,prediction[i][2]) == 0):
				ex2.write('{} {}\n'.format(i+1,word))


word_acc = [x/len(actual) for x in word_acc]
m = sum(len(word) for word in actual)
char_acc = [x/m for x in char_acc]
print('char_acc',char_acc)
print('word_acc',word_acc)