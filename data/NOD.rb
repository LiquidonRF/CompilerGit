a = 12

b = 486

while a != 0 && b != 0 do
    if a > b do
        a = a % b
    else
        b = b % a
    end
end

puts a + b
