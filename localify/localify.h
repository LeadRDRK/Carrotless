#ifndef UMAMUSUMELOCALIFYANDROID_LOCALIFY_H
#define UMAMUSUMELOCALIFYANDROID_LOCALIFY_H

#include "../il2cpp/il2cpp_symbols.h"

namespace localify {
    struct TextData
	{
		// { category: { index: text } }
		std::unordered_map<std::size_t, std::unordered_map<std::size_t, std::string>> Data;
	};

	struct CharacterSystemTextData
	{
		// { character_id: { voice_id: text } }
		std::unordered_map<std::size_t, std::unordered_map<std::size_t, std::string>> Data;
	};
	
	struct RaceJikkyoCommentData
	{
		// { id: text }
		std::unordered_map<std::size_t, std::string> Data;
	};

	struct RaceJikkyoMessageData
	{
		// { id: text }
		std::unordered_map<std::size_t, std::string> Data;
	};

    std::u16string u8_u16(const std::string &u8);

    std::string u16_u8(const std::u16string &u16);

    std::wstring u8_wide(const std::string &str);

    std::wstring u16_wide(const std::u16string &str);

    std::string wide_u8(const std::wstring &wstr);

    void load_textdb(const std::string &version, const std::vector<std::string> *dicts);

    void load_textId_textdb(const std::string &dict);

    void load_dicts();

    // void reload_textdb(const std::vector<std::string>* dicts);
    bool localify_text(size_t hash, std::string **result);

    Il2CppString *get_localized_string(size_t hash_or_id);

    Il2CppString *get_localized_string(Il2CppString *str);

    Il2CppString *get_localized_string(const std::string &textIdName);

    Il2CppString* GetTextData(std::size_t category, std::size_t index);

    Il2CppString* GetCharacterSystemTextData(std::size_t characterId, std::size_t voiceId);

    Il2CppString* GetRaceJikkyoCommentData(std::size_t id);

    Il2CppString* GetRaceJikkyoMessageData(std::size_t id);
}

#endif //UMAMUSUMELOCALIFYANDROID_LOCALIFY_H
