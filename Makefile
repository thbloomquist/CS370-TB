#
# Makefile for a CS 370 project
# Do not modify this file!!!
#

zip : clean solution.zip

solution.zip : collected-files.txt
	@echo "Creating a solution zip file"
	@zip -9 $@ `cat collected-files.txt`
	@rm -f collected-files.txt

# Collect the names of all files that don't appear
# to be generated files.
collected-files.txt :
	@echo "Collecting the names of files to be submitted"
	@rm -f $@
	@find . \( \
				-maxdepth 3 -name '*\.cpp' \
				-or -name '*\.h' \
				-or -name 'CMakeLists.txt' \
				-or -name '*\.vert' \
				-or -name '*\.frag' \
				-or -name '*\.obj' \
				-or -name '*\.jpg' \
				-or -name '*\.png' \
				-or -name 'Makefile' \
				-or -name '*\.pl' \
			\) -print \
		| perl -ne 's,\./,,; print' \
		> $@

# This is just a dummy target to force other targets
# to always be out of date.
nonexistent :

# Remove generated files.
clean : 
	rm -f collected-files.txt submit.properties solution.zip