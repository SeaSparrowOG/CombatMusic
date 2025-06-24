#include "Hooks/hooks.h"

namespace Hooks {
	bool Install() {
		logger::info("Installing hooks..."sv);
		return true;
	}
}