from pydantic import BaseModel, Field
from typing import Dict

class Position(BaseModel):
    x: int = Field(..., ge=0)
    y: int = Field(..., ge=0)

class CanvasConfig(BaseModel):
    width: int = Field(..., ge=100)
    height: int = Field(..., ge=100)


class Room(BaseModel):
    x: int = Field(..., ge=0)
    y: int = Field(..., ge=0)
    width: int = Field(..., ge=1)
    height: int = Field(..., ge=1)

class Config(BaseModel):
    beacons: Dict[str, Position]
    canvas: CanvasConfig
    room: Room
