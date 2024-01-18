#pragma once

#include <nlohmann/json.hpp>

/* json file example
{
  "projectName": "YourProjectName",
  "scenes": [
    {
      "uuid": 12301,
      "name": "example_name",
      "entities": [
        {
          "uuid": 423942305235235235,
          "transformComponent": {
            "rotation": [0, 0, 0],
            "scale": [1, 1, 1]
            // Add other transformation properties as needed
          },
          "materialComponent": {
            "reflectivity": 0.5,
            // Add other material properties as needed
          }
        }
        // Add more entities within the scene as needed
      ]
    },
    // Add more scenes as needed
  ]
}

*/


namespace TooGoodEngine {

	using json = nlohmann::json;

	class SceneSerializing
	{
	public:

	private:
	};
}