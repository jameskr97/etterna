/* Define all of the input devices we know about. This is the public interface
 * for describing input devices. */

#include "Etterna/Globals/global.h"
#include "Etterna/Models/Misc/LocalizedString.h"
#include "RageInputDevice.h"
#include "RageUtil/Utils/RageUtil.h"

#include <map>
#include <any>

static const char* InputDeviceStateNames[] = {
	"Connected",
	"Unplugged",
	"NeedsMultitap",
	"NoInputHandler",
};
XToString(InputDeviceState);
XToLocalizedString(InputDeviceState);
LuaXType(InputDevice);

//static std::map<std::any, std::string, decltype(cmp)> g_mapNamesToString;

static std::map<std::string, std::any> g_mapNamesToString;
//static std::map<std::string, DeviceButton> g_mapStringToNames;
static void InitNames() {
	if (!g_mapNamesToString.empty())
		return;

	g_mapNamesToString["period"] = Core::Input::Keys::Period;
	g_mapNamesToString["comma"] = Core::Input::Keys::Comma;
	g_mapNamesToString["colon"] = Core::Input::Keys::Semicolon;
	g_mapNamesToString["space"] = Core::Input::Keys::Space;
	g_mapNamesToString["delete"] = Core::Input::Keys::Delete;
	g_mapNamesToString["backslash"] = Core::Input::Keys::Backslash;

	g_mapNamesToString["backspace"] = Core::Input::Keys::Backspace;
	g_mapNamesToString["tab"] = Core::Input::Keys::Tab;
	g_mapNamesToString["enter"] = Core::Input::Keys::Enter;
	g_mapNamesToString["pause"] = Core::Input::Keys::Pause;
	g_mapNamesToString["escape"] = Core::Input::Keys::Escape;

	g_mapNamesToString["F1"] = Core::Input::Keys::F1;
	g_mapNamesToString["F2"] = Core::Input::Keys::F2;
	g_mapNamesToString["F3"] = Core::Input::Keys::F3;
	g_mapNamesToString["F4"] = Core::Input::Keys::F4;
	g_mapNamesToString["F5"] = Core::Input::Keys::F5;
	g_mapNamesToString["F6"] = Core::Input::Keys::F6;
	g_mapNamesToString["F7"] = Core::Input::Keys::F7;
	g_mapNamesToString["F8"] = Core::Input::Keys::F8;
	g_mapNamesToString["F9"] = Core::Input::Keys::F9;
	g_mapNamesToString["F10"] = Core::Input::Keys::F10;
	g_mapNamesToString["F11"] = Core::Input::Keys::F11;
	g_mapNamesToString["F12"] = Core::Input::Keys::F12;
	g_mapNamesToString["F13"] = Core::Input::Keys::F13;
	g_mapNamesToString["F14"] = Core::Input::Keys::F14;
	g_mapNamesToString["F15"] = Core::Input::Keys::F15;
	g_mapNamesToString["F16"] = Core::Input::Keys::F16;

	g_mapNamesToString["left ctrl"] = Core::Input::Keys::LeftControl;
	g_mapNamesToString["right ctrl"] = Core::Input::Keys::RightControl;
	g_mapNamesToString["left shift"] = Core::Input::Keys::LeftShift;
	g_mapNamesToString["right shift"] = Core::Input::Keys::RightShift;
	g_mapNamesToString["left alt"] = Core::Input::Keys::LeftAlt;
	g_mapNamesToString["right alt"] = Core::Input::Keys::RightAlt;
	// Note: On Windows, the Super key is the Windows key. -aj
	g_mapNamesToString["left super"] = Core::Input::Keys::LeftSuper;
	g_mapNamesToString["right super"] = Core::Input::Keys::RightSuper;
	g_mapNamesToString["menu"] = Core::Input::Keys::Menu;

	g_mapNamesToString["num lock"] = Core::Input::Keys::NumLock;
	g_mapNamesToString["scroll lock"] = Core::Input::Keys::ScrollLock;
	g_mapNamesToString["caps lock"] = Core::Input::Keys::CapsLock;
	g_mapNamesToString["prtsc"] = Core::Input::Keys::PrintScreen;

	g_mapNamesToString["up"] = Core::Input::Keys::Up;
	g_mapNamesToString["down"] = Core::Input::Keys::Down;
	g_mapNamesToString["left"] = Core::Input::Keys::Left;
	g_mapNamesToString["right"] = Core::Input::Keys::Right;

	g_mapNamesToString["insert"] = Core::Input::Keys::Insert;
	g_mapNamesToString["home"] = Core::Input::Keys::Home;
	g_mapNamesToString["end"] = Core::Input::Keys::End;
	g_mapNamesToString["pgup"] = Core::Input::Keys::PageUp;
	g_mapNamesToString["pgdn"] = Core::Input::Keys::PageDown;

	g_mapNamesToString["KP 0"] = Core::Input::Keys::KP0;
	g_mapNamesToString["KP 1"] = Core::Input::Keys::KP1;
	g_mapNamesToString["KP 2"] = Core::Input::Keys::KP2;
	g_mapNamesToString["KP 3"] = Core::Input::Keys::KP3;
	g_mapNamesToString["KP 4"] = Core::Input::Keys::KP4;
	g_mapNamesToString["KP 5"] = Core::Input::Keys::KP5;
	g_mapNamesToString["KP 6"] = Core::Input::Keys::KP6;
	g_mapNamesToString["KP 7"] = Core::Input::Keys::KP7;
	g_mapNamesToString["KP 8"] = Core::Input::Keys::KP8;
	g_mapNamesToString["KP 9"] = Core::Input::Keys::KP9;
	g_mapNamesToString["KP /"] = Core::Input::Keys::KPDivide;
	g_mapNamesToString["KP *"] = Core::Input::Keys::KPMultiply;
	g_mapNamesToString["KP -"] = Core::Input::Keys::KPSubtract;
	g_mapNamesToString["KP +"] = Core::Input::Keys::KPAdd;
	g_mapNamesToString["KP ."] = Core::Input::Keys::KPDecimal;
	g_mapNamesToString["KP ="] = Core::Input::Keys::KPEqual;
	g_mapNamesToString["KP enter"] = Core::Input::Keys::KPEnter;

	g_mapNamesToString["Left1"] = JOY_LEFT;
	g_mapNamesToString["Right1"] = JOY_RIGHT;
	g_mapNamesToString["Up1"] = JOY_UP;
	g_mapNamesToString["Down1"] = JOY_DOWN;

	// Secondary sticks:
	g_mapNamesToString["Left2"] = JOY_LEFT_2;
	g_mapNamesToString["Right2"] = JOY_RIGHT_2;
	g_mapNamesToString["Up2"] = JOY_UP_2;
	g_mapNamesToString["Down2"] = JOY_DOWN_2;

	g_mapNamesToString["Z-Up"] = JOY_Z_UP;
	g_mapNamesToString["Z-Down"] = JOY_Z_DOWN;
	g_mapNamesToString["R-Up"] = JOY_ROT_UP;
	g_mapNamesToString["R-Down"] = JOY_ROT_DOWN;
	g_mapNamesToString["R-Left"] = JOY_ROT_LEFT;
	g_mapNamesToString["R-Right"] = JOY_ROT_RIGHT;
	g_mapNamesToString["ZR-Up"] = JOY_ROT_Z_UP;
	g_mapNamesToString["ZR-Down"] = JOY_ROT_Z_DOWN;
	g_mapNamesToString["H-Left"] = JOY_HAT_LEFT;
	g_mapNamesToString["H-Right"] = JOY_HAT_RIGHT;
	g_mapNamesToString["H-Up"] = JOY_HAT_UP;
	g_mapNamesToString["H-Down"] = JOY_HAT_DOWN;
	g_mapNamesToString["Aux1"] = JOY_AUX_1;
	g_mapNamesToString["Aux2"] = JOY_AUX_2;
	g_mapNamesToString["Aux3"] = JOY_AUX_3;
	g_mapNamesToString["Aux4"] = JOY_AUX_4;

	g_mapNamesToString["left mouse button"] = Core::Input::Mouse::ButtonLeft;
	g_mapNamesToString["right mouse button"] = Core::Input::Mouse::ButtonRight;
	g_mapNamesToString["middle mouse button"] = Core::Input::Mouse::ButtonMiddle;
	g_mapNamesToString["mousewheel up"] = MOUSE_WHEELUP;
	g_mapNamesToString["mousewheel down"] = MOUSE_WHEELDOWN;

//	for (auto& m : g_mapNamesToString)
//		g_mapStringToNames[m.second] = m.first;
}

/* Return a reversible representation of a DeviceButton. This is not affected
 * by InputDrivers, localization or the keyboard language. */
std::string DeviceButtonToString(DeviceButton key) {
	InitNames();

	return "FIXME";
	// Check the name map first to allow making names for keys that are inside
	// the ascii range. -Kyz
//	auto it = g_mapNamesToString.find(key);
//	if (it != g_mapNamesToString.end())
//		return it->second;

	// All printable ASCII except for uppercase alpha characters line up.
	if (key >= 33 && key < 127 && !(key >= 'A' && key <= 'Z'))
		return ssprintf("%c", key);

	if (key >= KEY_OTHER_0 && key < KEY_LAST_OTHER)
		return ssprintf("unk %i", key - KEY_OTHER_0);

	if (key >= JOY_BUTTON_1 && key <= JOY_BUTTON_32)
		return ssprintf("B%i", key - JOY_BUTTON_1 + 1);

	if (key >= MIDI_FIRST && key <= MIDI_LAST)
		return ssprintf("Midi %d", key - MIDI_FIRST);

	return "unknown";
}

DeviceButton StringToDeviceButton(const std::string& s) {
	InitNames();

	if (s.size() == 1)
		return (DeviceButton)s[0];

	int i;
	if (sscanf(s.c_str(), "unk %i", &i) == 1)
		return enum_add2(KEY_OTHER_0, i);

	if (sscanf(s.c_str(), "B%i", &i) == 1)
		return enum_add2(JOY_BUTTON_1, i - 1);

	if (sscanf(s.c_str(), "Midi %i", &i) == 1)
		return enum_add2(MIDI_FIRST, i);

	if (sscanf(s.c_str(), "Mouse %i", &i) == 1)
		return enum_add2(MOUSE_LEFT, i);

//	auto it = g_mapStringToNames.find(s);
//	if (it != g_mapStringToNames.end())
//		return it->second;

	return DeviceButton_Invalid;
}
LuaXType(DeviceButton);

static const char* InputDeviceNames[] = {
	"Key",	 "Joy1",  "Joy2",  "Joy3",	"Joy4",	 "Joy5",  "Joy6",  "Joy7",
	"Joy8",	 "Joy9",  "Joy10", "Joy11", "Joy12", "Joy13", "Joy14", "Joy15",
	"Joy16", "Joy17", "Joy18", "Joy19", "Joy20", "Joy21", "Joy22", "Joy23",
	"Joy24", "Joy25", "Joy26", "Joy27", "Joy28", "Joy29", "Joy30", "Joy31",
	"Joy32", "Pump1", "Pump2", "Midi",	"Mouse", "PIUIO",
};
XToString(InputDevice);
StringToX(InputDevice);

/* Return a reversible representation of a DeviceInput. This is not affected by
 * InputDrivers, localization or the keyboard language. */
std::string DeviceInput::ToString() const {
	if (device == InputDevice_Invalid)
		return std::string();

	std::string s = InputDeviceToString(device) + "_" + DeviceButtonToString(button);
	return s;
}

bool DeviceInput::FromString(const std::string& s) {
	char szDevice[32] = "";
	char szButton[32] = "";

	if (2 != sscanf(s.c_str(), "%31[^_]_%31[^_]", szDevice, szButton)) {
		device = InputDevice_Invalid;
		return false;
	}

	device = StringToInputDevice(szDevice);
	button = StringToDeviceButton(szButton);
	return true;
}

std::string DeviceInput::to_string() const {
    for (const auto& [mkey, value] : g_mapNamesToString){

        if(value.type() == typeid(Core::Input::Mouse)
            && this->mouse == *std::any_cast<Core::Input::Mouse>(&value)) {
            return "DeviceButton_" + mkey;
        }

        if(value.type() == typeid(Core::Input::Keys)
            && this->key == *std::any_cast<Core::Input::Keys>(&value)) {
            return "DeviceButton_" + mkey;
        }

    }
    return ""; // unknown
}
