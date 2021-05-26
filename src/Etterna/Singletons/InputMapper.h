#ifndef INPUT_MAPPER_H
#define INPUT_MAPPER_H

#include <utility>

#include <utility>

#include "Etterna/Models/Misc/GameInput.h"
#include "Etterna/Models/Misc/PlayerNumber.h"
#include "RageUtil/Misc/RageInputDevice.h"
struct Game;

const int NUM_GAME_TO_DEVICE_SLOTS =
  5; // five device inputs may map to one game input
const int NUM_SHOWN_GAME_TO_DEVICE_SLOTS = 3;
const int NUM_USER_GAME_TO_DEVICE_SLOTS = 2;
extern const std::string DEVICE_INPUT_SEPARATOR;

struct AutoMappingEntry {
	AutoMappingEntry(int i, DeviceButton db, GameButton gb, bool b)
	 : m_iSlotIndex(i), m_deviceButton(db), m_gb(gb), m_bSecondController(b) {}

    AutoMappingEntry(int i, Core::Input::Keys key, GameButton gb, bool b)
	 : m_iSlotIndex(i), key(key), m_gb(gb), m_bSecondController(b) {}
	AutoMappingEntry() = default;

	[[nodiscard]] auto IsEmpty() const -> bool {
	    return m_deviceButton == DeviceButton_Invalid && key == Core::Input::Keys::Unknown && m_gb == GameButton_Invalid;
	}

	int m_iSlotIndex{ -1 };
	DeviceButton m_deviceButton{ DeviceButton_Invalid };
	GameButton m_gb {GameButton_Invalid}; // GameButton_Invalid means unmap this button
	Core::Input::Keys key{Core::Input::Keys::Unknown};

	/* If this is true, this is an auxilliary mapping assigned to the second
	 * player. If two of the same device are found, and the device has secondary
	 * entries, the later entries take precedence. This way, if a Pump pad is
	 * found, it'll map P1 to the primary pad and P2 to the secondary pad. (We
	 * can't tell if a slave pad is actually there.) Then, if a second primary
	 * is found (DEVICE_PUMP2), 2P will be mapped to it. */
	bool m_bSecondController{ false };
};

struct AutoMappings {
	AutoMappings(std::string s1, std::string s2, std::string s3, std::initializer_list<AutoMappingEntry> list = {})
	  : m_sGame(std::move(std::move(s1))), m_sDriverRegex(std::move(std::move(s2))), m_sControllerName(std::move(std::move(s3)))
	{

        for (const auto &item : list){
            if(!item.IsEmpty())
	            m_vMaps.emplace_back(item);
        }

	}

	// Strings used by automatic joystick mappings.
	std::string m_sGame;		   // only used
	std::string m_sDriverRegex;	   // reported by InputHandler
	std::string m_sControllerName; // the product name of the controller

	std::vector<AutoMappingEntry> m_vMaps;
};

class InputScheme
{
  public:
	const char* m_szName;
	int m_iButtonsPerController;
	struct GameButtonInfo
	{
		const char* m_szName; // The name used by the button graphics system.
							  // e.g. "left", "right", "middle C", "snare"
		GameButton m_SecondaryMenuButton;
	};
	// Data for each Game-specific GameButton. This starts at GAME_BUTTON_NEXT.
	GameButtonInfo m_GameButtonInfo[NUM_GameButton];
	const AutoMappings* m_pAutoMappings;

	[[nodiscard]] auto ButtonNameToIndex(const std::string& sButtonName) const -> GameButton;
	[[nodiscard]] auto GameButtonToMenuButton(GameButton gb) const -> GameButton;
	void MenuButtonToGameInputs(GameButton MenuI, PlayerNumber pn, std::vector<GameInput>& GameIout) const;
	void MenuButtonToGameButtons(GameButton MenuI, std::vector<GameButton>& aGameButtons) const;
	[[nodiscard]] auto GetGameButtonInfo(GameButton gb) const -> const GameButtonInfo*;
	[[nodiscard]] auto GetGameButtonName(GameButton gb) const -> const char*;
};
/** @brief A special foreach loop to handle the various GameButtons. */
#define FOREACH_GameButtonInScheme(s, var)                                     \
	for (GameButton var = (GameButton)0; (var) < (s)->m_iButtonsPerController; \
		 enum_add<GameButton>(var, +1))

class InputMappings
{
  public:
	// only filled for automappings
	std::string m_sDeviceRegex;
	std::string m_sDescription;

	// map from a GameInput to multiple DeviceInputs
	DeviceInput m_GItoDI[NUM_GameController][NUM_GameButton][NUM_GAME_TO_DEVICE_SLOTS];

	void Clear();
	void Unmap(InputDevice id);
	void WriteMappings(const InputScheme* pInputScheme, const std::string& sFilePath);
	void ReadMappings(const InputScheme* pInputScheme, const std::string& sFilePath, bool bIsAutoMapping);
	void SetInputMap(const DeviceInput& DeviceI, const GameInput& GameI, int iSlotIndex);

	void ClearFromInputMap(const DeviceInput& DeviceI);
	auto ClearFromInputMap(const GameInput& GameI, int iSlotIndex) -> bool;
};
/** @brief Holds user-chosen input preferences and saves it between sessions. */
class InputMapper
{
  public:
	InputMapper();
	~InputMapper();
	void PushSelf(lua_State* L);

	void SetInputScheme(const InputScheme* pInputScheme);
	[[nodiscard]] auto GetInputScheme() const -> const InputScheme*;
	void SetJoinControllers(PlayerNumber pn);

	void ReadMappingsFromDisk();
	void SaveMappingsToDisk();
	void ResetMappingsToDefault();
	void CheckButtonAndAddToReason(GameButton menu, std::vector<std::string>& full_reason, std::string const& sub_reason);
	void SanityCheckMappings(std::vector<std::string>& reason);

	void ClearAllMappings();

	void SetInputMap(const DeviceInput& DeviceI, const GameInput& GameI, int iSlotIndex);
	void ClearFromInputMap(const DeviceInput& DeviceI);
	auto ClearFromInputMap(const GameInput& GameI, int iSlotIndex) -> bool;

	void AddDefaultMappingsForCurrentGameIfUnmapped();
	void AutoMapJoysticksForCurrentGame();
	auto CheckForChangedInputDevicesAndRemap(std::string& sMessageOut) -> bool;

	[[nodiscard]] auto IsMapped(const DeviceInput& DeviceI) const -> bool;

	auto DeviceToGame(const DeviceInput& DeviceI, GameInput& GameI) const -> bool; // return true if there is a mapping from device to pad
	auto GameToDevice(const GameInput& GameI, int iSlotNum, DeviceInput& DeviceI) const -> bool; // return true if there is a mapping from pad to device

	[[nodiscard]] auto GameButtonToMenuButton(GameButton gb) const -> GameButton;
	void MenuToGame(GameButton MenuI, PlayerNumber pn, std::vector<GameInput>& GameIout) const;
	[[nodiscard]] auto ControllerToPlayerNumber(GameController controller) const -> PlayerNumber;

	[[nodiscard]] auto GetSecsHeld(const GameInput& GameI, MultiPlayer mp = MultiPlayer_Invalid) const -> float;
	[[nodiscard]] auto GetSecsHeld(GameButton MenuI, PlayerNumber pn) const -> float;

	auto IsBeingPressed(const GameInput& GameI, MultiPlayer mp = MultiPlayer_Invalid, const DeviceInputList* pButtonState = nullptr) const -> bool;
	[[nodiscard]] auto IsBeingPressed(GameButton MenuI, PlayerNumber pn) const -> bool;
	auto IsBeingPressed(const std::vector<GameInput>& GameI, MultiPlayer mp = MultiPlayer_Invalid, const DeviceInputList* pButtonState = nullptr) const -> bool;

	void ResetKeyRepeat(const GameInput& GameI);
	void ResetKeyRepeat(GameButton MenuI, PlayerNumber pn);

	void RepeatStopKey(const GameInput& GameI);
	void RepeatStopKey(GameButton MenuI, PlayerNumber pn);

	[[nodiscard]] auto GetLevel(const GameInput& GameI) const -> float;
	[[nodiscard]] auto GetLevel(GameButton MenuI, PlayerNumber pn) const -> float;

	static auto MultiPlayerToInputDevice(MultiPlayer mp) -> InputDevice;
	static auto InputDeviceToMultiPlayer(InputDevice id) -> MultiPlayer;

	void Unmap(InputDevice device);
	void ApplyMapping(const std::vector<AutoMappingEntry>& vMmaps, GameController gc, InputDevice id);

  protected:
	InputMappings m_mappings;

	void UpdateTempDItoGI();
	const InputScheme* m_pInputScheme;

  private:
	InputMapper(const InputMapper& rhs);
	auto operator=(const InputMapper& rhs) -> InputMapper&;
};

extern InputMapper* INPUTMAPPER; // global and accessible from anywhere in our program

#endif
