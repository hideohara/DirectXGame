#include "Model.h"
#include <DirectXTex.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

/// <summary>
/// 静的メンバ変数の実体
/// </summary>
ID3D12Device* Model::sDevice = nullptr;
UINT Model::sDescriptorHandleIncrementSize = 0;
ID3D12GraphicsCommandList* Model::sCommandList = nullptr;
ComPtr<ID3D12RootSignature> Model::sRootSignature;
ComPtr<ID3D12PipelineState> Model::sPipelineState;

void Model::StaticInitialize(ID3D12Device* device, int window_width, int window_height) {
  // nullptrチェック
  assert(device);

  sDevice = device;

  // パイプライン初期化
  InitializeGraphicsPipeline();
}

void Model::PreDraw(ID3D12GraphicsCommandList* commandList) {
  // PreDrawとPostDrawがペアで呼ばれていなければエラー
  assert(Model::sCommandList == nullptr);

  // コマンドリストをセット
  sCommandList = commandList;

  // パイプラインステートの設定
  commandList->SetPipelineState(sPipelineState.Get());
  // ルートシグネチャの設定
  commandList->SetGraphicsRootSignature(sRootSignature.Get());
  // プリミティブ形状を設定
  commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Model::PostDraw() {
  // コマンドリストを解除
  sCommandList = nullptr;
}

Model* Model::Create() {
  // 3Dオブジェクトのインスタンスを生成
  Model* object3d = new Model();
  assert(object3d);

  // 初期化
  object3d->Initialize();

  return object3d;
}

void Model::InitializeGraphicsPipeline() {
  HRESULT result = S_FALSE;
  ComPtr<ID3DBlob> vsBlob;    // 頂点シェーダオブジェクト
  ComPtr<ID3DBlob> psBlob;    // ピクセルシェーダオブジェクト
  ComPtr<ID3DBlob> errorBlob; // エラーオブジェクト

  // 頂点シェーダの読み込みとコンパイル
  result = D3DCompileFromFile(
    L"Resources/shaders/BasicVS.hlsl", // シェーダファイル名
    nullptr,
    D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
    "main", "vs_5_0", // エントリーポイント名、シェーダーモデル指定
    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
    0, &vsBlob, &errorBlob);
  if (FAILED(result)) {
	// errorBlobからエラー内容をstring型にコピー
	std::string errstr;
	errstr.resize(errorBlob->GetBufferSize());

	std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
	errstr += "\n";
	// エラー内容を出力ウィンドウに表示
	OutputDebugStringA(errstr.c_str());
	exit(1);
  }

  // ピクセルシェーダの読み込みとコンパイル
  result = D3DCompileFromFile(
    L"Resources/shaders/BasicPS.hlsl", // シェーダファイル名
    nullptr,
    D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
    "main", "ps_5_0", // エントリーポイント名、シェーダーモデル指定
    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
    0, &psBlob, &errorBlob);
  if (FAILED(result)) {
	// errorBlobからエラー内容をstring型にコピー
	std::string errstr;
	errstr.resize(errorBlob->GetBufferSize());

	std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
	errstr += "\n";
	// エラー内容を出力ウィンドウに表示
	OutputDebugStringA(errstr.c_str());
	exit(1);
  }

  // 頂点レイアウト
  D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
    {// xy座標(1行で書いたほうが見やすい)
     "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {// 法線ベクトル(1行で書いたほうが見やすい)
     "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {// uv座標(1行で書いたほうが見やすい)
     "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT,
     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  };

  // グラフィックスパイプラインの流れを設定
  D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
  gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
  gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

  // サンプルマスク
  gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定
  // ラスタライザステート
  gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  // gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
  // gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
  //  デプスステンシルステート
  gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

  // レンダーターゲットのブレンド設定
  D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
  blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // RBGA全てのチャンネルを描画
  blenddesc.BlendEnable = true;
  blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
  blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
  blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

  blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
  blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;
  blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;

  // ブレンドステートの設定
  gpipeline.BlendState.RenderTarget[0] = blenddesc;

  // 深度バッファのフォーマット
  gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

  // 頂点レイアウトの設定
  gpipeline.InputLayout.pInputElementDescs = inputLayout;
  gpipeline.InputLayout.NumElements = _countof(inputLayout);

  // 図形の形状設定（三角形）
  gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

  gpipeline.NumRenderTargets = 1;                            // 描画対象は1つ
  gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 0～255指定のRGBA
  gpipeline.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング

  // デスクリプタレンジ
  CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
  descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 レジスタ

  // ルートパラメータ
  CD3DX12_ROOT_PARAMETER rootparams[3] = {};
  rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
  rootparams[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
  rootparams[2].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

  // スタティックサンプラー
  CD3DX12_STATIC_SAMPLER_DESC samplerDesc =
    CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

  // ルートシグネチャの設定
  CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
  rootSignatureDesc.Init_1_0(
    _countof(rootparams), rootparams, 1, &samplerDesc,
    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  ComPtr<ID3DBlob> rootSigBlob;
  // バージョン自動判定のシリアライズ
  result = D3DX12SerializeVersionedRootSignature(
    &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
  // ルートシグネチャの生成
  result = sDevice->CreateRootSignature(
    0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),
    IID_PPV_ARGS(&sRootSignature));
  assert(SUCCEEDED(result));

  gpipeline.pRootSignature = sRootSignature.Get();

  // グラフィックスパイプラインの生成
  result = sDevice->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&sPipelineState));
  assert(SUCCEEDED(result));
}

void Model::CreateMesh() {
  HRESULT result = S_FALSE;

  vertices_ = {
  //  x      y      z       nx     ny    nz       u     v
  // 前
    {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}}, // 左下
    {{-1.0f, +1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}}, // 左上
    {{+1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}}, // 右下
    {{+1.0f, +1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}}, // 右上
  // 後(前面とZ座標の符号が逆)
    {{+1.0f, -1.0f, +1.0f}, {0.0f, 0.0f, +1.0f}, {0.0f, 1.0f}}, // 左下
    {{+1.0f, +1.0f, +1.0f}, {0.0f, 0.0f, +1.0f}, {0.0f, 0.0f}}, // 左上
    {{-1.0f, -1.0f, +1.0f}, {0.0f, 0.0f, +1.0f}, {1.0f, 1.0f}}, // 右下
    {{-1.0f, +1.0f, +1.0f}, {0.0f, 0.0f, +1.0f}, {1.0f, 0.0f}}, // 右上
  // 左
    {{-1.0f, -1.0f, +1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // 左下
    {{-1.0f, +1.0f, +1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 左上
    {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // 右下
    {{-1.0f, +1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // 右上
  // 右（左面とX座標の符号が逆）
    {{+1.0f, -1.0f, -1.0f}, {+1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // 左下
    {{+1.0f, +1.0f, -1.0f}, {+1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 左上
    {{+1.0f, -1.0f, +1.0f}, {+1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // 右下
    {{+1.0f, +1.0f, +1.0f}, {+1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // 右上
  // 下
    {{+1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}, // 左下
    {{+1.0f, -1.0f, +1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, // 左上
    {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}}, // 右下
    {{-1.0f, -1.0f, +1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}, // 右上
  // 上（下面とY座標の符号が逆）
    {{-1.0f, +1.0f, -1.0f}, {0.0f, +1.0f, 0.0f}, {0.0f, 1.0f}}, // 左下
    {{-1.0f, +1.0f, +1.0f}, {0.0f, +1.0f, 0.0f}, {0.0f, 0.0f}}, // 左上
    {{+1.0f, +1.0f, -1.0f}, {0.0f, +1.0f, 0.0f}, {1.0f, 1.0f}}, // 右下
    {{+1.0f, +1.0f, +1.0f}, {0.0f, +1.0f, 0.0f}, {1.0f, 0.0f}}, // 右上
  };

  // 頂点インデックスの設定
  indices_ = {0,  1,  3,  3,  2,  0,

              4,  5,  7,  7,  6,  4,

              8,  9,  11, 11, 10, 8,

              12, 13, 15, 15, 14, 12,

              16, 17, 19, 19, 18, 16,

              20, 21, 23, 23, 22, 20};

  // 頂点データのサイズ
  UINT sizeVB = static_cast<UINT>(sizeof(VertexPosNormalUv) * vertices_.size());

  {
	// ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	// リソース設定
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeVB);

	// 頂点バッファ生成
	result = sDevice->CreateCommittedResource(
	  &heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
	  IID_PPV_ARGS(&vertBuff_));
	assert(SUCCEEDED(result));
  }

  // インデックスデータのサイズ
  UINT sizeIB = static_cast<UINT>(sizeof(uint16_t) * indices_.size());

  {
	// ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	// リソース設定
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeIB);

	// インデックスバッファ生成
	result = sDevice->CreateCommittedResource(
	  &heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
	  IID_PPV_ARGS(&indexBuff_));
	assert(SUCCEEDED(result));
  }

  // 頂点バッファへのデータ転送
  VertexPosNormalUv* vertMap = nullptr;
  result = vertBuff_->Map(0, nullptr, (void**)&vertMap);
  if (SUCCEEDED(result)) {
	std::copy(vertices_.begin(), vertices_.end(), vertMap);
	vertBuff_->Unmap(0, nullptr);
  }

  // インデックスバッファへのデータ転送
  uint16_t* indexMap = nullptr;
  result = indexBuff_->Map(0, nullptr, (void**)&indexMap);
  if (SUCCEEDED(result)) {
	std::copy(indices_.begin(), indices_.end(), indexMap);

	indexBuff_->Unmap(0, nullptr);
  }

  // 頂点バッファビューの作成
  vbView_.BufferLocation = vertBuff_->GetGPUVirtualAddress();
  vbView_.SizeInBytes = sizeVB;
  vbView_.StrideInBytes = sizeof(vertices_[0]);

  // インデックスバッファビューの作成
  ibView_.BufferLocation = indexBuff_->GetGPUVirtualAddress();
  ibView_.Format = DXGI_FORMAT_R16_UINT;
  ibView_.SizeInBytes = sizeIB;
}

void Model::Initialize() {
  // nullptrチェック
  assert(sDevice);

  // メッシュ生成
  CreateMesh();
}

void Model::Draw(
  const WorldTransform& worldTransform, const ViewProjection& viewProjection,
  uint32_t textureHadle) {
  // nullptrチェック
  assert(sDevice);
  assert(sCommandList);
  assert(worldTransform.constBuff_.Get());

  // 頂点バッファの設定
  sCommandList->IASetVertexBuffers(0, 1, &vbView_);
  // インデックスバッファの設定
  sCommandList->IASetIndexBuffer(&ibView_);

  // CBVをセット（ワールド行列）
  sCommandList->SetGraphicsRootConstantBufferView(
    static_cast<UINT>(RoomParameter::kWorldTransform),
    worldTransform.constBuff_->GetGPUVirtualAddress());

  // CBVをセット（ビュープロジェクション行列）
  sCommandList->SetGraphicsRootConstantBufferView(
    static_cast<UINT>(RoomParameter::kViewProjection),
    viewProjection.constBuff_->GetGPUVirtualAddress());

  // SRVをセット
  TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(
    sCommandList, static_cast<UINT>(RoomParameter::kTexture), textureHadle);

  // 描画コマンド
  sCommandList->DrawIndexedInstanced(static_cast<UINT>(indices_.size()), 1, 0, 0, 0);
}
