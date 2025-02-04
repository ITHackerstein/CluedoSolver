#include "ErrorModal.hpp"

#include "../LanguageStrings.hpp"
#include "../utils/IconsFontAwesome.h"

#include <imgui.h>

namespace Cluedo {

namespace UI {

void ErrorModal::show() {
	if (ImGui::BeginPopupModal(CSTR(LS("UI.Error")), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::PushStyleColor(ImGuiCol_Text, 0xCA0B00FF);
		ImGui::AlignTextToFramePadding();
		ImGui::TextColored({ 0.79f, 0.04f, 0.00f, 1.00f }, ICON_FA_TRIANGLE_EXCLAMATION);
		ImGui::PopStyleColor();

		ImGui::SameLine();
		ImGui::Text("%s", m_error_message.c_str());

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::Button(CSTR(LS("UI.Ok")))) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

}

}
