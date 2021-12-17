FLAGS=-Wall -Wextra

make:
	gcc $(FLAGS) diag.c -o diag -fopenmp -lm
	gcc $(FLAGS) create.c -o create -fopenmp
	gcc $(FLAGS) mat.h -o mat.o

test: make
	@echo "Testing with sddm!"
	@echo "------------------"
	-@./create     --sddm 2> /dev/null | ./diag || ( tput setaf 1; echo     'TEST FAILED sddm!'; tput setaf 15 )
	@echo "Testing with non-sddm!"
	@echo "----------------------"
	-@./create --not-sddm 2> /dev/null | ./diag && ( tput setaf 1; echo 'TEST FAILED non-sddm!'; tput setaf 15 )
	# cat 16_000.mat | ./diag 1 >> results.txt
	# cat 16_000.mat | ./diag 4 >> results.txt

run1: make
	./create --sddm 10 | ./diag 1
	./create --sddm 10 | ./diag 2
	./create --sddm 10 | ./diag 4
	./create --sddm 10 | ./diag 8

render:
	R --quiet -e "require(rmarkdown);render('report.rmd');"

submit:
	cp report.pdf 171014.pdf
	zip Askisi_1_171014.zip 171014.pdf create.c diag.c mat.h bench.py

.PHONY: render
