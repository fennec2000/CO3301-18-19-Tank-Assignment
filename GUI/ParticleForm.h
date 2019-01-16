#pragma once

namespace ParticleTool {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for ParticleForm
	/// </summary>
	public ref class ParticleForm : public System::Windows::Forms::Form
	{
	public:
		ParticleForm(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~ParticleForm()
		{
			if (components)
			{
				delete components;
			}
		}
	public: System::Windows::Forms::Panel^  renderPanel;
	private: System::Windows::Forms::Button^  exitButton;
	private: System::Windows::Forms::Button^  resetButton;
	private: System::Windows::Forms::TrackBar^  trackBar1;
	private: System::Windows::Forms::Label^  label1;


	private:

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->renderPanel = (gcnew System::Windows::Forms::Panel());
			this->exitButton = (gcnew System::Windows::Forms::Button());
			this->resetButton = (gcnew System::Windows::Forms::Button());
			this->trackBar1 = (gcnew System::Windows::Forms::TrackBar());
			this->label1 = (gcnew System::Windows::Forms::Label());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar1))->BeginInit();
			this->SuspendLayout();
			// 
			// renderPanel
			// 
			this->renderPanel->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->renderPanel->Location = System::Drawing::Point(12, 12);
			this->renderPanel->Name = L"renderPanel";
			this->renderPanel->Size = System::Drawing::Size(1084, 814);
			this->renderPanel->TabIndex = 0;
			// 
			// exitButton
			// 
			this->exitButton->Location = System::Drawing::Point(1225, 13);
			this->exitButton->Name = L"exitButton";
			this->exitButton->Size = System::Drawing::Size(75, 23);
			this->exitButton->TabIndex = 1;
			this->exitButton->Text = L"Exit";
			this->exitButton->UseVisualStyleBackColor = true;
			this->exitButton->Click += gcnew System::EventHandler(this, &ParticleForm::exitButton_Click);
			// 
			// resetButton
			// 
			this->resetButton->Location = System::Drawing::Point(1225, 42);
			this->resetButton->Name = L"resetButton";
			this->resetButton->Size = System::Drawing::Size(75, 23);
			this->resetButton->TabIndex = 2;
			this->resetButton->Text = L"Reset";
			this->resetButton->UseVisualStyleBackColor = true;
			this->resetButton->Click += gcnew System::EventHandler(this, &ParticleForm::resetButton_Click);
			// 
			// trackBar1
			// 
			this->trackBar1->LargeChange = 1000;
			this->trackBar1->Location = System::Drawing::Point(1102, 71);
			this->trackBar1->Maximum = 200000;
			this->trackBar1->Minimum = 100;
			this->trackBar1->Name = L"trackBar1";
			this->trackBar1->Size = System::Drawing::Size(198, 45);
			this->trackBar1->SmallChange = 100;
			this->trackBar1->TabIndex = 3;
			this->trackBar1->TickFrequency = 10000;
			this->trackBar1->Value = 100000;
			this->trackBar1->ValueChanged += gcnew System::EventHandler(this, &ParticleForm::trackBar1_ValueChanged);
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12));
			this->label1->Location = System::Drawing::Point(1103, 123);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(131, 20);
			this->label1->TabIndex = 4;
			this->label1->Text = L"Particles: 100000";
			// 
			// ParticleForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1312, 838);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->trackBar1);
			this->Controls->Add(this->resetButton);
			this->Controls->Add(this->exitButton);
			this->Controls->Add(this->renderPanel);
			this->Name = L"ParticleForm";
			this->Text = L"ParticleForm";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar1))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

		private: System::Void exitButton_Click(System::Object^  sender, System::EventArgs^  e)
		{
			Close();
		}
		private: System::Void resetButton_Click(System::Object^  sender, System::EventArgs^  e)
		{
			int value = 100000;
		}
		private: System::Void trackBar1_ValueChanged(System::Object^  sender, System::EventArgs^  e)
		{
		}
	};
}
