CHPL:=chpl

.PHONY: all
all: matrix_mul

matrix_mul: matrix_mul.chpl
	$(CHPL) -o $@ $<


.PHONY: clean
clean:
	$(RM) matrix_mul

