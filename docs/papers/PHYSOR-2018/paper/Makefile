all: paper.pdf

paper.pdf: paper.tex
	latexmk -g -pdf paper.tex

.PHONY: clean

clean:
	latexmk -C
	rm -f *~ .*~
