// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the corona store (see TweaksVCoronaStore.h).

#include "TweaksVCoronaStore.h"

namespace TweaksV
{
	bool FindCoronaStore(uint8_t*, uint8_t*, std::vector<CodeWrite>&, std::string&)
	{
		return true;
	}

	CoronaStoreNumbers CoronaStoreNow()
	{
		return { kGameCoronas, false, 0, 0, 0 };
	}
}
