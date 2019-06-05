all: stat_methods.pdf

stat_methods.pdf : stat_methods.tex
	pdflatex stat_methods.tex
	pdflatex stat_methods.tex
	makeindex stat_methods.idx
	pdflatex stat_methods.tex
	pdflatex stat_methods.tex

clean:
	rm -f *~ *.log *.aux *.dvi *.bak *.ind *.idx *.toc *.ilg *.out
