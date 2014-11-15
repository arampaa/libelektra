import QtQuick 2.2

WizardTemplate {

	wizardText.text: qsTr("Welcome to the Mounting Wizard! Here you can create and mount backends." +
						  "\n\nPlease provide a unique name.\n\nAlready used are: " + usedNames)

	label.text: qsTr("Backend name: ")
	usedNames: " default" + (externTreeModel.mountedBackends().toString() === "empty" ? "" : ", " + externTreeModel.mountedBackends().toString().replace(",", ", "))

	buttonRow.backButton.enabled: false
	buttonRow.finishButton.enabled: false
	buttonRow.nextButton.onClicked: loader.source = "Page2.qml"
}
