
.PHONY = all clean

all : benchmark.pdf

benchmark_TEXFILES = benchmark.tex

DIAGGPFILES = test-benchmark-N-btime-c60000-n1-v1-o6-o7-I1.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o6-o7-I10.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o6-o7-I100.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o28-o29-I1.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o30-o31-I1.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o32-o33-I1.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o34-o35-I1.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o36-o37-I1.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o38-o39-I1.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o40-o41-I1.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o42-o43-I1.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o28-o29-I10.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o30-o31-I10.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o32-o33-I10.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o34-o35-I10.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o36-o37-I10.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o38-o39-I10.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o40-o41-I10.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o42-o43-I10.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o28-o29-I100.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o30-o31-I100.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o32-o33-I100.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o34-o35-I100.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o36-o37-I100.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o38-o39-I100.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o40-o41-I100.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o42-o43-I100.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o44-o45-I1.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o46-o47-I1.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o48-o49-I1.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o50-o51-I1.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o52-o53-I1.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o54-o55-I1.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o56-o57-I1.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o58-o59-I1.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o44-o45-I10.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o46-o47-I10.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o48-o49-I10.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o50-o51-I10.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o52-o53-I10.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o54-o55-I10.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o56-o57-I10.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o58-o59-I10.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o44-o45-I100.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o46-o47-I100.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o48-o49-I100.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o50-o51-I100.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o52-o53-I100.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o54-o55-I100.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o56-o57-I100.diag.gp \
			  test-benchmark-N-btime-c60000-n1-v1-o58-o59-I100.diag.gp

benchmark.pdf : $(DIAGGPFILES:.gp=.tex) $(benchmark_TEXFILES)
	pdflatex ./benchmark.tex
	pdflatex ./benchmark.tex
	pdflatex ./benchmark.tex

%.diag.tex : %.diag.gp
	gnuplot $< > $@

clean :
	rm -fr *.aux \
	rm -fr *.bbl \
	rm -fr *.blg \
	rm -fr *.diag.tex \
	rm -fr *.fig.bak \
	rm -fr *-gfx.pdf \
	rm -fr *.idx \
	rm -fr *.ilg \
	rm -fr *.ind \
	rm -fr *.lof \
	rm -fr *.log \
	rm -fr *.lol \
	rm -fr *.lot \
	rm -fr *.nav \
	rm -fr *.out \
	rm -fr *.pdf \
	rm -fr *.png \
	rm -fr *.snm \
	rm -fr *.toc \
	rm -fr *.vrb

