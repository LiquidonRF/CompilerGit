stack = "searching string index"
search = "ing"

i = 0
j = 0
count = 0

n = len(stack)
m = len(search)

while i < n do
	if stack[i] == search[j] do
		j = j + 1
		count = count + 1
	else
		j = 0
		count = 0
	end
	
	if m == count do
		puts i
		i = n
	end

	i = i + 1
end
