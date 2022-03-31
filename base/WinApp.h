#pragma once
#include <Windows.h>

/// <summary>
/// ウィンドウズアプリケーション
/// </summary>
class WinApp {
public: // 静的メンバ変数
  // ウィンドウサイズ
  static const int kWindowWidth = 1280; // 横幅
  static const int kWindowHeight = 720; // 縦幅
  // ウィンドウクラス名
  static const wchar_t kWindowClassName[];

public: // 静的メンバ関数
  /// <summary>
  /// ウィンドウプロシージャ
  /// </summary>
  /// <param name="hwnd">ウィンドウハンドル</param>
  /// <param name="msg">メッセージ番号</param>
  /// <param name="wparam">メッセージ情報1</param>
  /// <param name="lparam">メッセージ情報2</param>
  /// <returns>成否</returns>
  static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public: // メンバ関数
  /// <summary>
  /// ゲームウィンドウの作成
  /// </summary>
  void CreateGameWindow();

  /// <summary>
  /// ゲームウィンドウの破棄
  /// </summary>
  void TerminateGameWindow();

  /// <summary>
  /// メッセージの処理
  /// </summary>
  /// <returns>終了かどうか</returns>
  bool ProcessMessage();

  /// <summary>
  /// ウィンドウハンドルの取得
  /// </summary>
  /// <returns></returns>
  HWND GetHwnd() { return hwnd_; }

  HINSTANCE GetInstance() { return wndClass_.hInstance; }

private: // メンバ変数
  // Window関連
  HWND hwnd_ = nullptr;   // ウィンドウハンドル
  WNDCLASSEX wndClass_{}; // ウィンドウクラス
};
