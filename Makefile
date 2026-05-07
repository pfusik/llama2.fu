all: c cpp cs d java js py

c cpp java js py:
	$(MAKE) -C $@

cs:
	dotnet build cs -c Release

d:
	dub build --root=d -b release

.PHONY: c cpp cs d java js py
