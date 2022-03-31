#include "DebugText.h"
#include "TextureManager.h"

DebugText::DebugText() {}

DebugText::~DebugText() {
  for (int i = 0; i < _countof(spriteDatas_); i++) {
	delete spriteDatas_[i];
  }
}

void DebugText::Initialize() {

  // デバッグテキスト用テクスチャ読み込み
  textureHandle_ = TextureManager::Load("debugfont.png");
  // 全てのスプライトデータについて
  for (int i = 0; i < _countof(spriteDatas_); i++) {
	// スプライトを生成する
	spriteDatas_[i] = Sprite::Create(textureHandle_, {0, 0});
  }
}

// 1文字列追加
void DebugText::Print(const std::string& text, float x, float y, float scale = 1.0f) {
  // 全ての文字について
  for (int i = 0; i < text.size(); i++) {
	// 最大文字数超過
	if (spriteIndex_ >= kMaxCharCount) {
	  break;
	}

	// 1文字取り出す(※ASCIIコードでしか成り立たない)
	const unsigned char& character = text[i];

	int fontIndex = character - 32;
	if (character >= 0x7f) {
	  fontIndex = 0;
	}

	int fontIndexY = fontIndex / kFontLineCount;
	int fontIndexX = fontIndex % kFontLineCount;

	// 座標計算
	spriteDatas_[spriteIndex_]->SetPosition({x + kFontWidth * scale * i, y});
	spriteDatas_[spriteIndex_]->SetTextureRect(
	  {(float)fontIndexX * kFontWidth, (float)fontIndexY * kFontHeight},
	  {(float)kFontWidth, (float)kFontHeight});
	spriteDatas_[spriteIndex_]->SetSize({kFontWidth * scale, kFontHeight * scale});

	// 文字を１つ進める
	spriteIndex_++;
  }
}

// まとめて描画
void DebugText::DrawAll(ID3D12GraphicsCommandList* cmdList) {
  // 全ての文字のスプライトについて
  for (int i = 0; i < spriteIndex_; i++) {
	// スプライト描画
	spriteDatas_[i]->Draw();
  }

  spriteIndex_ = 0;
}