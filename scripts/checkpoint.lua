checkpoint = {}

function checkpoint.OnTouch(id, player)
  curMap = entity.new(realizing().activeMap)

  if not player:playable().checkpointMapObject or
      not curMap:mappable().mapId == player:playable().checkpointMapId or
      not id:prototypable().mapObjectIndex ==
        player:playable().checkpointMapObjectIndex then

    player:playable().checkpointMapObject = true
    player:playable().checkpointMapId = curMap:mappable().mapId
    player:playable().checkpointMapObjectIndex =
      id:prototypable().mapObjectIndex
    player:playable().checkpointPos = player:transformable().pos

    playSound("res/Pickup_Coin23.wav", 0.25)
  end
end
