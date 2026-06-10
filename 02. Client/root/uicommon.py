# Add anywhere in the RewardListDialog class:
if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
	class MountUpGradeDialog(QuestionDialog):
		def __init__(self):
			QuestionDialog.__init__(self)
			self.__CreateDialog()

		def __del__(self):
			QuestionDialog.__del__(self)

		def __CreateDialog(self):
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "uiscript/mount_up_grade_dialog.py")

			self.board = self.GetChild("board")
			self.textLine1 = self.GetChild("message1")
			self.textLine2 = self.GetChild("message2")
			self.acceptButton = self.GetChild("accept")
			self.cancelButton = self.GetChild("cancel")

		def SetText1(self, text):
			self.textLine1.SetText(text)

		def SetText2(self, text):
			self.textLine2.SetText(text)

		def Open(self):
			self.SetCenterPosition()
			self.SetTop()
			self.Show()

		def SetAcceptEvent(self, event):
			if app.ENABLE_GAME_OPTION_ESCAPE:
				self.accept_event_func = event
			self.acceptButton.SetEvent(event)

		def SetCancelEvent(self, event):
			if app.ENABLE_GAME_OPTION_ESCAPE:
				self.cancel_event_func = event
			self.cancelButton.SetEvent(event)
