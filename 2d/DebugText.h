#pragma once

#include "Sprite.h"
#include <Windows.h>
#include <string>

/// <summary>
/// デバッグ用文字表示
/// </summary>
class DebugText {
public:
  // デバッグテキスト用のテクスチャ番号を指定
  static const int kMaxCharCount = 256; // 最大文字数
  static const int kFontWidth = 9;      // フォント画像内1文字分の横幅
  static const int kFontHeight = 18;    // フォント画像内1文字分の縦幅
  static const int kFontLineCount = 14; // フォント画像内1行分の文字数

  DebugText();
  ~DebugText();

  void Initialize();

  void Print(const std::string& text, float x, float y, float size);

  void DrawAll(ID3D12GraphicsCommandList* cmdList);

private:
  uint32_t textureHandle_ = 0;
  // スプライトデータの配列
  Sprite* spriteDatas_[kMaxCharCount] = {};
  // スプライトデータ配列の添え字番号
  int spriteIndex_ = 0;
};
