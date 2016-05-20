BUILD_DIR=build

#MAKE_FLAGS=-j 1
MAKE_FLAGS=-j 4
#
TEST_FLAGS=-j 50 -V

.PHONY: all debug release doc clean test

all:
	cd $(BUILD_DIR) && $(MAKE) $(MAKE_FLAGS) || echo "Type either \"make debug\" or \"make release\"!"

$(BUILD_DIR): debug

debug:
	cd $(BUILD_DIR) && cmake -B$(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug .. && $(MAKE) $(MAKE_FLAGS)

release:
	cd $(BUILD_DIR) && cmake -B$(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release .. && $(MAKE) $(MAKE_FLAGS)

doc:
	cd $(BUILD_DIR) && $(MAKE) $(MAKE_FLAGS) doc

test: $(BUILD_DIR)
	cd $(BUILD_DIR) && ctest $(TEST_FLAGS)

clean:
	cd $(BUILD_DIR) && rm -rf *
	rm -rf html

