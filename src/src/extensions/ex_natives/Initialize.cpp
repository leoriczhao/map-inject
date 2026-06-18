// Initialize.cpp — Entry point for EX* native registration
// Ported from YDWE yd_jass_api/Initialize.cpp
// Calls all sub-initializers to register EX* functions via jass::japi_add().

#include <windows.h>

namespace warcraft3::japi {

	void InitializeUnitState();
	void InitializeAbilityState();
	void InitializeItemState();
	void InitializeEventDamageData();
	void InitializeDisplayChat();
	void InitializeEffect();

	void initialize_ex_natives()
	{
		InitializeUnitState();
		InitializeAbilityState();
		InitializeItemState();
		InitializeEventDamageData();
		InitializeDisplayChat();
		InitializeEffect();
	}
}
