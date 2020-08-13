// charAt(int) // returns string

var house = "house";
console.log(house.charAt(2));

// concat(st1, ...) -> returns newString

var str1 = "Hello";
var str2 = "World!";
str1 = str1.concat(" ",str2)
console.log(str1)

// endsWith(searchVal, length~) -> returns bool

var str1 = "Hello World"
console.log(str1.endsWith("World"))

// includes(searchVal, start~) -> returns bool

var str = "Hello World";
console.log(str.includes(str.charAt(6)));

// 