checkpoint = {}

function checkpoint.OnTouch(id, player)
  curMap = entity.new(realizing():singleton():realizable().activeMap)

  if not player:playable().checkpointMapObject or
      not curMap:mappable().mapId == player:playable().checkpointMapId or
      not id:prototypable().mapObjectIndex ==
        player:playable().checkpointMapObjectIndex then

    player:playable().checkpointMapObject = true
    player:playable().checkpointMapId = curMap:mappable().mapId
    player:playable().checkpointMapObjectIndex =
      id:prototypable().mapObjectIndex
    player:playable().checkpointPos = player:transformable().pos
  end
end
