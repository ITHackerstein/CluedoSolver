#include "PlayerDataModal.hpp"

#include "../LanguageStrings.hpp"

#include <fmt/format.h>
#include <imgui.h>

namespace Cluedo {

namespace UI {

void PlayerDataModal::show(Solver const& solver) {
	if (ImGui::BeginPopupModal(CSTR(LS("UI.PlayerData")), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		for (size_t i = 0; i < solver.player_count(); ++i) {
			auto const& player = solver.player(i);
			ImGui::TextUnformatted(fmt::format("{} - {} {}", player.name(), player.card_count(), LS("UI.Cards")).c_str());
		}

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
