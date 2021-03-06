# Taken largely from http://scottmcpeak.com/autodepend/autodepend.html
# I mean to mess with another build systems (maybe scons) at some point,
# but will do just fine until then

CXXFLAGS := -std=c++11 -Wall -Wextra -Wconversion -Weffc++ -pedantic -DWITH_JSON
LIBFLAGS := -pthread -ljsoncpp -lboost_system

OBJS := $(patsubst %.cpp,%.o, $(wildcard common/*.cpp))
TESTOBJS := $(patsubst %.cpp,%.o, $(wildcard tests/*.cpp))

debug: CXXFLAGS += -g
debug: gallery

unit_tests: CXXFLAGS += -I. -Icommon -Itests -g
unit_tests: $(OBJS) $(TESTOBJS)
	echo $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(TESTOBJS) $(LIBFLAGS) -o unit_tests

release: CXXFLAGS+= -O2 -flto -DNDEBUG
debug: gallery

# link
gallery: $(OBJS) main.o
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBFLAGS) main.o -o gallery

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
	rm -f tests/*.o tests/*.d common/*.o common/*.d *.o *.gch *.d

.PHONY: clean debug release
