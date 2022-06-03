DOC_NAME=SAND2011-UserManual

TEX_SECTIONS=\
	abstract.tex \
	introduction.tex \
	featureFamilyProcessing.tex \
	topologyFileFormat.tex \
	installation.tex \
	usage.tex \
	SANDdistribution.tex

all: $(DOC_NAME).pdf

$(DOC_NAME).pdf: $(DOC_NAME).tex $(TEX_SECTIONS) $(DOC_NAME).bbl
	pdflatex $(DOC_NAME)
	thumbpdf $(DOC_NAME)
	pdflatex $(DOC_NAME)

$(DOC_NAME).bbl: $(DOC_NAME).bib $(GENERATED_FIGURES)
	pdflatex $(DOC_NAME)
	bibtex $(DOC_NAME)
	pdflatex $(DOC_NAME)

clean::
	rm -f $(DOC_NAME).pdf *~ *.bbl *.blg *.toc *.ilg \
	*.ind *.idx *.lof *.log *.lot *.aux *.out *.tpt 
	rm -fR pdf


png/%.png: svg/%.svg png
	inkscape --export-png=$@ -d 200 --export-background=ffffffff $<

png:
	mkdir png
