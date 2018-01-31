import json

api_url = input("Home API URL: ")
api_key = input("API Key: ")
system_json = input("System")
		
def addSystem() {
	system = json.load(open("system.json"))

	rooms = system["rooms"]
	devices = system["devices"]
	actions = system["actions"]
	conditions = system["conditions"]

	# Add rooms (this is probably not needed)
	for room in rooms:
		uuid_old = room["uuid"]
		addRoom(room)
		# Update room_id in devices
		for device in devices:
			if device["room_id"] == uuid_old:
				device["room_id"] = room["uuid"]

	# Add devices (and params)
	for device in devices:
		# query db, check if device type already exist
		# ask user if add new device or use existing
		# ask device name, room

		params_old = device["parameters"]
		addDevice(device)
		params_new = device["parameters"]
		# Update paramIDs in actions and conditions
		for param_old in params_old:
			param_new = next(p for p in params_new if p["paramName"] == param_old["paramName"])
			# update actions
			for action in actions:
				for actionCommand in action["actionCommands"]:
					if actionCommand["paramID"] == param_old["uuid"]:
						actionCommand["paramID"] = param_new["uuid"]
			# update conditions
			for condition in conditions:
				if condition["paramID"] == param_old["uuid"]
					condition["paramID"] = param_new["uuid"]
				if condition["comparisonParameter"] == param_old["uuid"]
					condition["comparisonParameter"] = param_new["uuid"]
	# Add actions
	for action in actions:
		uuid_old = action["uuid"]
		addAction(action)
		# Update actionID in conditions
		for condition in conditions:
			if condition["actionID"] == uuid_old:
				condition["actionID"] = uuid_old

	# Add conditions
	for condition in conditions:
		addCondition()
}