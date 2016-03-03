# tmarkdown

TMarkdown is a command line tool that generates html pages from markdown files.
A header and footer html file can be added to customize the html generated.

Usage:

tmarkdon input.md output.htm -h header.txt -f footer.txt

tmarkdon -all -h header.txt -f footer.txt
(all .md files will be generated)

The tmarkdown suports:

```
# Headers using this format

**bold** and __italic__ using this format

cpp source code
\```cpp
\```

Pre
\```
\```

* lists

links and images and lines


```
