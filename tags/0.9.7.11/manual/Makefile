html-pdf: web pdf

# we can't name the target "html", since there is already a folder with that name
# and so make thinks this target is already up to date
web:
	docbook2html -d qtiplot.dsl docbook-en/index.docbook -e no-valid

pdf:
	dblatex -o qtiplot-manual-en.pdf -c dblatex.conf -p dblatex-param.xsl docbook-en/index.docbook
