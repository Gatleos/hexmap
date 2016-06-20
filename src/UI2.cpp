#include "UI2.h"
#include "imgui_internal.h"
#include "HexMap.h"
#include "ResourceLoader.h"
#include <sstream>

namespace ui2 {
	void drawAnim(AnimHandler& anim, ImVec2& canvasSize, ImColor& borderColor = ImColor(0, 0, 0, 0)) {
		const sf::Texture* tex = &anim.getAnimationData().getTexture();
		const sf::VertexArray& vert = anim.getVArray();
		sf::Vector2u size = tex->getSize();
		//
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 origin = ImGui::GetCursorScreenPos();
		ImVec2 canvasCenter(canvasSize.x / 2.0f, canvasSize.y / 2.0f);
		draw_list->PushClipRect(origin, ImVec2(origin.x + canvasSize.x + 2, origin.y + canvasSize.y + 2));
		// draw each rect in the current frame of the animation
		for (int vIndex = 0; vIndex < vert.getVertexCount(); vIndex += 4) {
			auto &topLeft = vert[vIndex];
			auto &bottomRight = vert[vIndex+2];
			draw_list->AddImage((void*)tex, ImVec2(origin.x + topLeft.position.x + canvasCenter.x + 1, origin.y + topLeft.position.y + canvasCenter.y + 1),
				ImVec2(origin.x + bottomRight.position.x + canvasCenter.x + 1, origin.y + bottomRight.position.y + canvasCenter.y + 1),
				ImVec2(topLeft.texCoords.x / size.x, topLeft.texCoords.y / size.y),
				ImVec2(bottomRight.texCoords.x / size.x, bottomRight.texCoords.y / size.y));
		}
		draw_list->AddRect(ImVec2(origin.x, origin.y), ImVec2(origin.x + canvasSize.x + 2, origin.y + canvasSize.y + 2), borderColor);
		// move the cursor
		draw_list->PopClipRect();
		auto curPos = ImGui::GetCursorPos();
		ImGui::ItemSize(ImRect(origin.x, origin.y, origin.x + canvasSize.x, origin.y + canvasSize.y));
		//ImGui::SetCursorPos(ImVec2(curPos.x, curPos.y + canvasSize.y + 2));
	}
	void mapInfoMenu(sf::Vector2i& mouseMapPos, sf::Vector2i& tilePos, sf::Vector2i& viewCenter, std::string tileName, int& genTime, bool& randomSeed, AnimHandler& anim) {
		stringstream ss;
		const int columnWidth = 100.0f;
		static char* seedBuf = new char[9];
		seedBuf[0] = 0;
		if (!ImGui::Begin("Map Info", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize)) {
			ImGui::End();
			return;
		}
		ImGui::BeginChild((ImGuiID)0, {200.0f, 110.0f});
		// camPos
		ss << viewCenter.x << "," << viewCenter.y;
		ImGui::Text("Camera:"); ImGui::SameLine(columnWidth); ImGui::Text(ss.str().c_str());
		ss.str(std::string());
		// mousePos
		ss << mouseMapPos.x << "," << mouseMapPos.y;
		ImGui::Text("Mouse:"); ImGui::SameLine(columnWidth); ImGui::Text(ss.str().c_str());
		ss.str(std::string());
		// tilePos
		ss << tilePos.x << "," << tilePos.y;
		ImGui::Text("Hex:"); ImGui::SameLine(columnWidth); ImGui::Text(ss.str().c_str());
		ss.str(std::string());
		// tileName
		ImGui::Text("Terrain:"); ImGui::SameLine(columnWidth); ImGui::Text(tileName.c_str());
		// genTime
		ImGui::Text("Gen (ms):"); ImGui::SameLine(columnWidth); ImGui::Text(std::to_string(genTime).c_str());
		// chunks
		const sf::IntRect& ir = HEXMAP.getChunkViewArea();
		ss << "(" << ir.left << ", " << ir.top << " / " << ir.width << ", " << ir.height << ")";
		ImGui::Text("Chunks:"); ImGui::SameLine(columnWidth); ImGui::Text(ss.str().c_str());
		ImGui::Separator();
		ImGui::EndChild();
		//
		ImGui::Checkbox("Random seed", &randomSeed); ImGui::SameLine(); ImGui::InputText("", seedBuf, 9, ImGuiInputTextFlags_CharsHexadecimal);
		drawAnim(anim, ImVec2(10.0f, 50.0f), ImColor(0, 255, 255));
		ImGui::SameLine(); ImGui::Button("Generate"); ImGui::SameLine(); ImGui::Button("Reload files");
		ImGui::SameLine(); ImGui::Text("aw yiss");
		ImGui::End();
	}
}