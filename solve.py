import sys

True = 1
False = 0

def load(f):
	board = [0] * 81
	i = 0
	for line in open(f):
		for c in line.rstrip():
			if c <= '9' and c >= '1':
				board[i] = ord(c) - ord('0')
			elif c == 'x':
				board[i] = 0
			else:
				raise Exception, "Unknown character: " + c
			i += 1
		if i % 9 != 0:
			raise Exception, "Strange format: " + line
	if i != 81:
		raise Exception, "Expected 81 characters, got %d" % i
	return board

def hasduplicates(candidates):
	known = {}
	for c in candidates:
		if c > 0:
		 	if c in known:
				return True
			known[c] = 1
	return False

def unused(candidates, choices=None):
	if choices is None:
		choices = dict([(choice, 1) for choice in range(1, 10)])
	for c in candidates:
		if c in choices:
			del choices[c]
	return choices

def legal(board):
	for i in range(9):
		if hasduplicates(board[i*9:(i+1)*9]):
			return False
		elif hasduplicates([board[j*9 + i] for j in range(9)]):
			return False
		elif hasduplicates([board[(j // 3 + i // 3 * 3) * 9 + j % 3 + i % 3 * 3] for j in range(9)]):
			return False
	return True

def legalmoves(board, index):
	row = index // 9
	choices = unused([board[index // 9 * 9 + i] for i in range(9)])
	if not choices:
		return []
	choices = unused([board[index % 9 + i * 9] for i in range(9)], choices)
	if not choices:
		return []
	choices = unused([board[(j // 3 + index // 27 * 3) * 9 + j % 3 + index % 9 // 3 * 3] for j in range(9)], choices)
	return choices.keys()

def unfilled(board):
	return len([b for b in board if b == 0])

def findnext(board):
	best = (10, None, None)
	for i in range(len(board)):
		if board[i] == 0:
			l = legalmoves(board, i)
			if len(l) <= 1:
				return i, l
			elif len(l) < best[0]:
				best = (len(l), i, l)
	return best[1], best[2]

def solve(board, checklegal=True):
	if checklegal:
		if not legal(board):
			pretty(board)
			raise Exception, "Not legal board"
	index, choices = findnext(board)
	if index is None:
		return True, board
 	for choice in choices:
		board[index] = choice
		correct, solution = solve(board, False)
		if correct:
			return correct, solution
		board[index] = 0
	return False, None

def pretty(board):
	s = ""
	for i in range(len(board)):
		s += "%d" % board[i]
		if i % 9 == 8:
			s += "\n"
		elif i % 3 == 2:
			s += " "
	return s

def main():
	import sys
	board = load(sys.argv[1])
	correct, solution = solve(board)
	if correct:
		print "Solved"
		print pretty(solution)
	else:
		print "Solution not found"

if __name__ == '__main__':
	main()
#	import profile
#	profile.run("main()", "prof.out")

