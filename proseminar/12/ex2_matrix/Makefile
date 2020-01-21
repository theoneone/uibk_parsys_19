CHPL:=chpl

.PHONY: all
all: matrix_mul_dist

matrix_mul_dist: matrix_mul_dist.chpl
	$(CHPL) -o $@ $<


.PHONY: clean
	$(RM) matrix_mul_dist

