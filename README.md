# brainduck
 
<b>brainduck</b> is an esoteric programming language based on Brainfuck. 

## Compilation

You sould compile ``brainduckc.c`` and use it like compiler 

```shell
gcc brainduckc.c -o brainduckc
./brainduckc main.bd [-o OutputPath] [-m MemSize]
```

By default compile to ``a.out``. Can be configured by ``-o OutputPath``

## Syntax

### Initial position

By default ``MemSize = 30'000``. Can be configured by ``-m MemSize``

Start ``position = 0``

There must be a ``main`` function in your code.

### Comments

You can use comment line:
```
//some comment
```
Or comment block:
```
/* line 0
   line 1
   line 2 */
```

#### Operations

``>`` - Increase position

``<`` - Decrease position

``+`` - Increase memory current position

``-`` - Decrease memory current position

``,`` - Read char and put it to memory current position. If EOF - return 0. 

This operation is equal to ``int _ = getchar(); mem[pos] = (_ == -1 ? 0 : _)`` in C.

``.`` - Write char from memory current position. This operation is equal to ``putchar(mem[pos])`` in C.

``[`` - If memory current position equal zero - go to the end bracket.

``]`` - Go to the start bracket.

``{`` - If memory current position equal zero - go to the end bracket.

``}`` - End bracket.

### Subtasks

You can create subtasks using syntax:
```
name: code
```
For example:
```
plus10: ++++++++++
plus11: plus10|+
```

#### Default Subtasks

``n`` - Print EOL. Equal ``putchar('\n')`` in C.

``feof`` - Check EOF in stdin. Equel ``mem[pos] = feof(stdin)`` in C. 

#### Subtasks execution

You can call subtasks using syntax:
```
name|
```

## examples

Read char, write it and write EOL:
```
main:,.n|
```

Write all chars from 1 to 255:
```
main:+[.+]
```

Read string until ``\0``, print it reversed and print EOL:
```
main:>,[>,]<[.<]n|
```

## Notes

``,`` will read every char.

If you needn't command line arguments, you can clear them by [``clearArgs``](https://github.com/aleks5d/brainduck/blob/main/examples/clear_args.bd) function.
