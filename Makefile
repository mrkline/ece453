# Taken largely from http://scottmcpeak.com/autodepend/autodepend.html
# I mean to mess with another build systems (maybe scons) at some point,
# but will do just fine until then

CXXFLAGS := -std=c++11 -Wall -Wextra -Weffc++ -pedantic

OBJS = $(filter-out common/WinsockRequirement.o, $(patsubst %.cpp,%.o, $(wildcard common/*.cpp)))
TESTOBJS = $(patsubst %.cpp,%.o, $(wildcard tests/*.cpp))

unit_tests: CXXFLAGS += -I. -Icommon -Itests -g
unit_tests: $(OBJS) $(TESTOBJS)
	echo $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(TESTOBJS) -o unit_tests

# debug: CXXFLAGS += -g

# release: CXXFLAGS+= -O2 -DNDEBUG

# link
cstyle: $(OBJS) main.o
	$(CXX) $(CXXFLAGS) $(OBJS) main.o -o cstyle

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)
-include $(TESTOBJS:.o=.d)

# For if we used precomipled headers later
# precomp.hpp.gch: precomp.hpp
# 	$(CXX) $(CXXFLAGS) precomp.hpp

# compile and generate dependency info;
# more complicated dependency computation, so all prereqs listed
# will also become command-less, prereq-less targets
#   sed:    strip the target (everything before colon)
#   sed:    remove any continuation backslashes
#   fmt -1: list words one per line
#   sed:    strip leading spaces
#   sed:    add trailing colons
# %.o: precomp.hpp.gch %.cpp
%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $*.cpp -o $*.o
	$(CXX) -MM $(CXXFLAGS) $*.cpp > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

# remove compilation products
clean:
	rm -f tests/*.o common/*.o *.o *.gch *.d

.PHONY: clean debug release
