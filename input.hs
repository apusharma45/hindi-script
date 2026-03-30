likho "=== HindiScript feature showcase ==="
likho "enter a number for n:"

rakho n me padho()
likho "you entered (n):"
likho n

rakho a me 10
rakho b me 3
rakho c me (a + b) * 2 - -4
likho "a ="
likho a
likho "b ="
likho b
likho "c = (a + b) * 2 - -4"
likho c

likho "cmp a > b ="
likho a > b
likho "cmp a < b ="
likho a < b
likho "cmp a >= 10 ="
likho a >= 10
likho "cmp b <= 3 ="
likho b <= 3
likho "cmp a == 10 ="
likho a == 10
likho "cmp b != 3 ="
likho b != 3

agar c > 20 to shuru
    likho "if: c > 20"
khatam
warna shuru
    likho "else: c <= 20"
khatam

jab n >= 90 to shuru
    likho "grade: A"
khatam
anya jab n >= 75 to shuru
    likho "grade: B"
khatam
anya shuru
    likho "grade: C"
khatam

banao mul x y shuru
    wapas x * y
khatam

banao mix x y z shuru
    rakho tmp me mul(x, y)
    wapas tmp + z
khatam

banao next x shuru
    wapas x + 1
khatam

rakho m me mix(2, 5, 7)
likho "m = mix(2, 5, 7)"
likho m

rakho p me 5 |> mul(3) |> next()
likho "p = 5 |> mul(3) |> next()"
likho p

rakho i me 1
rakho total me 0
jabtak i <= 5 shuru
    rakho total me total + i
    rakho i me i + 1
khatam
likho "total = sum 1..5"
likho total

likho "done"
