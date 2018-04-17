all: manual.pdf

manual.pdf: manual.tex
	latexmk -g -pdf manual.tex

.PHONY: clean

clean:
	latexmk -C
	rm -f *~ .*~
