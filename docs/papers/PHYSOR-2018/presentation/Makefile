all: physor.pdf

physor.pdf: physor.tex physor.bib
	latexmk -g -pdf physor.tex
	bibtex physor
	latexmk -g -pdf physor.tex
	latexmk -g -pdf physor.tex

.PHONY: clean

clean:
	latexmk -C
	rm -f *~ .*~
