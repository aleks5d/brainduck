# brainduck
 
<b>brainduck</b> is an esoteric programming language based on Brainfuck. 

## compilation

you sould compile brainduckc.c and use it like compiler 

```shell
gcc brainduckc.c -o brainduckc
./brainduckc main.bd
```

Current version compile file to a.out

## syntax

### initial position

current version has a memory tape of size = 30'000

stat position = 0

### comments

you can use comment line 
```
//some comment
```
or comment block
```
/* line 0
   line 1
   line 2 */
```

#### operations

```
>
```
increase position
```
<
```
decrease position
```
+
```
increase memory current position
```
-
```
decrease memory current position
```
,
```
read char and put it to memory current position
```
.
```
write char from memory current position
```
[
```
if memory current position equal zero - go to the end bracket 
```
]
```
go to the start bracket

### subtasks

```
n
```
\print '\n'

syntax of subtasks calling:
```
name|
```

## examples

```
,.n|
```
will read input char, write it and write '\n'

```
+[.+]
```
will write all chars form 1 to 255

```
>,[>,]<[.<]
```

will read string until char(0) and print it reversed. 

