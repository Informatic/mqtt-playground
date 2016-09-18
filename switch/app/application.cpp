#include <SpejsNode.h>

SpejsNode node("switch");

void init() {
	node.init();
	node.registerEndpoint("relay", new OutputEndpoint(5));
}
