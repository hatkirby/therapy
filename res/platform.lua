function moveLeft(entity, len, speed)
  local remaining = len / speed

  while (remaining > 0) do
    print("no")
    --entity:ponderable().vel:x(-speed)
    remaining = remaining - coroutine.yield()
  end
end

function moveRight(entity, len, speed)
  local remaining = len / speed

  while (remaining > 0) do
    print("no2")
    --entity:ponderable().vel:x(speed)
    remaining = remaining - coroutine.yield()
  end
end

function run(entity)
  print("yes")
  while true do
    moveRight(entity, 90, 30)
    moveLeft(entity, 90, 30)
  end
end