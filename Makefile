all: c cpp cs java js py

c cpp java js py:
	$(MAKE) -C $@

cs:
	dotnet build cs -c Release

.PHONY: c cpp cs java js py
