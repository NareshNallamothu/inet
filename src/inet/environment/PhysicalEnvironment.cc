//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/environment/PhysicalEnvironment.h"
#include "inet/common/geometry/Box.h"
#include "inet/common/geometry/Cuboid.h"
#include "inet/common/geometry/Sphere.h"
#include "inet/common/geometry/polytope/Face.h"
#include "inet/common/geometry/Prism.h"
#include "inet/common/geometry/polytope/ConvexPolytope.h"
#include "inet/environment/Material.h"
#include "inet/common/geometry/Rotation.h"
#include <algorithm>

namespace inet {

Define_Module(PhysicalEnvironment);

PhysicalEnvironment::PhysicalEnvironment() :
    temperature(sNaN),
    spaceMin(Coord(sNaN, sNaN, sNaN)),
    spaceMax(Coord(sNaN, sNaN, sNaN)),
    objectCache(NULL),
    objectsLayer(NULL)
{
}

PhysicalEnvironment::~PhysicalEnvironment()
{
    for (std::vector<const Shape3D *>::iterator it = shapes.begin(); it != shapes.end(); it++)
        delete *it;
    for (std::vector<const Material *>::iterator it = materials.begin(); it != materials.end(); it++)
        delete *it;
    for (std::vector<const PhysicalObject *>::iterator it = objects.begin(); it != objects.end(); it++)
        delete *it;
}

void PhysicalEnvironment::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        objectCache = dynamic_cast<IObjectCache *>(getSubmodule("objectCache"));
        temperature = K(par("temperature"));
        spaceMin.x = par("spaceMinX");
        spaceMin.y = par("spaceMinY");
        spaceMin.z = par("spaceMinZ");
        spaceMax.x = par("spaceMaxX");
        spaceMax.y = par("spaceMaxY");
        spaceMax.z = par("spaceMaxZ");
        viewAngle = computeViewAngle(par("viewAngle"));
        viewRotation = Rotation(viewAngle);
        objectsLayer = new cGroupFigure();
        cCanvas *canvas = getParentModule()->getCanvas();
        canvas->addFigure(objectsLayer, canvas->findFigure("submodules"));
    }
    else if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT)
    {
        cXMLElement *environment = par("config");
        parseShapes(environment);
        parseMaterials(environment);
        parseObjects(environment);
    }
    else if (stage == INITSTAGE_LAST)
    {
        updateCanvas();
    }
}

void PhysicalEnvironment::parseShapes(cXMLElement *xml)
{
    cXMLElementList children = xml->getChildrenByTagName("shape");
    for (cXMLElementList::const_iterator it = children.begin(); it != children.end(); ++it)
    {
        cXMLElement *element = *it;
        Shape3D *shape = NULL;
        // id
        const char *idAttribute = element->getAttribute("id");
        int id = -1;
        if (idAttribute)
            id = atoi(idAttribute);
        // type
        const char *typeAttribute = element->getAttribute("type");
        if (!typeAttribute)
            throw cRuntimeError("Missing type attribute of shape");
        else if (!strcmp(typeAttribute, "cuboid"))
        {
            Coord size;
            const char *sizeAttribute = element->getAttribute("size");
            if (!sizeAttribute)
                throw cRuntimeError("Missing size attribute of cuboid");
            cStringTokenizer tokenizer(sizeAttribute);
            if (tokenizer.hasMoreTokens())
                size.x = atof(tokenizer.nextToken());
            if (tokenizer.hasMoreTokens())
                size.y = atof(tokenizer.nextToken());
            if (tokenizer.hasMoreTokens())
                size.z = atof(tokenizer.nextToken());
            shape = new Cuboid(size);
        }
        else if (!strcmp(typeAttribute, "sphere"))
        {
            double radius;
            const char *radiusAttribute = element->getAttribute("radius");
            if (!radiusAttribute)
                throw cRuntimeError("Missing radius attribute of sphere");
            else
                radius = atof(radiusAttribute);
            shape = new Sphere(radius);
        }
        else if (!strcmp(typeAttribute, "prism"))
        {
            double height;
            const char *heightAttribute = element->getAttribute("height");
            if (!heightAttribute)
                throw cRuntimeError("Missing height attribute of prism");
            else
                height = atof(heightAttribute);
            std::vector<Coord> points;
            const char *pointsAttribute = element->getAttribute("points");
            if (!pointsAttribute)
                throw cRuntimeError("Missing points attribute of prism");
            else {
                cStringTokenizer tokenizer(pointsAttribute);
                while (tokenizer.hasMoreTokens())
                {
                    Coord point;
                    if (tokenizer.hasMoreTokens())
                        point.x = atof(tokenizer.nextToken());
                    if (tokenizer.hasMoreTokens())
                        point.y = atof(tokenizer.nextToken());
                    points.push_back(point);
                }
            }
            shape = new Prism(height, Polygon(points));
        }
        else if (!strcmp(typeAttribute, "polytope"))
        {
            std::vector<Coord> points;
            const char *pointsAttribute = element->getAttribute("points");
            if (!pointsAttribute)
                throw cRuntimeError("Missing points attribute of polytope");
            else {
                cStringTokenizer tokenizer(pointsAttribute);
                while (tokenizer.hasMoreTokens())
                {
                    Coord point;
                    if (tokenizer.hasMoreTokens())
                        point.x = atof(tokenizer.nextToken());
                    if (tokenizer.hasMoreTokens())
                        point.y = atof(tokenizer.nextToken());
                    if (tokenizer.hasMoreTokens())
                        point.z = atof(tokenizer.nextToken());
                    points.push_back(point);
                }
            }
            shape = new ConvexPolytope(points);
        }
        else
            throw cRuntimeError("Unknown shape type '%s'", typeAttribute);
        // insert
        if (idToShapeMap.find(id) != idToShapeMap.end())
            throw cRuntimeError("Shape already exists with the same id: '%d'", id);
        idToShapeMap.insert(std::pair<int, const Shape3D *>(id, shape));
    }
}

void PhysicalEnvironment::parseMaterials(cXMLElement *xml)
{
    cXMLElementList children = xml->getChildrenByTagName("material");
    for (cXMLElementList::const_iterator it = children.begin(); it != children.end(); ++it)
    {
        cXMLElement *element = *it;
        // id
        const char *idAttribute = element->getAttribute("id");
        if (!idAttribute)
            throw cRuntimeError("Missing mandatory id attribute of material");
        int id = atoi(idAttribute);
        // name
        const char *name = element->getAttribute("name");
        // resistivity
        const char *resistivityAttribute = element->getAttribute("resistivity");
        if (!resistivityAttribute)
            throw cRuntimeError("Missing mandatory resistivity attribute of material");
        Ohmm resistivity = Ohmm(atof(resistivityAttribute));
        // relativePermittivity
        const char *relativePermittivityAttribute = element->getAttribute("relativePermittivity");
        if (!relativePermittivityAttribute)
            throw cRuntimeError("Missing mandatory relativePermittivity attribute of material");
        double relativePermittivity = atof(relativePermittivityAttribute);
        // relativePermeability
        const char *relativePermeabilityAttribute = element->getAttribute("relativePermeability");
        if (!relativePermeabilityAttribute)
            throw cRuntimeError("Missing mandatory relativePermeability attribute of material");
        double relativePermeability = atof(relativePermeabilityAttribute);
        // insert
        if (idToMaterialMap.find(id) != idToMaterialMap.end())
            throw cRuntimeError("Material already exists with the same id: '%d'", id);
        Material *material = new Material(name, resistivity, relativePermittivity, relativePermeability);
        materials.push_back(material);
        idToMaterialMap.insert(std::pair<int, const Material *>(id, material));
        nameToMaterialMap.insert(std::pair<const std::string, const Material *>(material->getName(), material));
    }
}

void PhysicalEnvironment::parseObjects(cXMLElement *xml)
{
    cXMLElementList children = xml->getChildren();
    for (cXMLElementList::const_iterator it = children.begin(); it != children.end(); ++it)
    {
        cXMLElement *element = *it;
        const char *tag = element->getTagName();
        if (strcmp(tag, "object"))
            continue;
        // id
        const char *idAttribute = element->getAttribute("id");
        int id = -1;
        if (idAttribute)
            id = atoi(idAttribute);
        // name
        const char *name = element->getAttribute("name");
        // orientation
        EulerAngles orientation;
        const char *orientationAttribute = element->getAttribute("orientation");
        if (orientationAttribute)
        {
            cStringTokenizer tokenizer(orientationAttribute);
            if (tokenizer.hasMoreTokens())
                orientation.alpha = math::deg2rad(atof(tokenizer.nextToken()));
            if (tokenizer.hasMoreTokens())
                orientation.beta = math::deg2rad(atof(tokenizer.nextToken()));
            if (tokenizer.hasMoreTokens())
                orientation.gamma = math::deg2rad(atof(tokenizer.nextToken()));
        }
        // shape
        const Shape3D *shape;
        const char *shapeAttribute = element->getAttribute("shape");
        if (!shapeAttribute)
            throw cRuntimeError("Missing shape attribute of object");
        cStringTokenizer shapeTokenizer(shapeAttribute);
        const char *shapeType = shapeTokenizer.nextToken();
        if (!strcmp(shapeType, "cuboid"))
        {
            Coord size;
            if (shapeTokenizer.hasMoreTokens())
                size.x = atof(shapeTokenizer.nextToken());
            if (shapeTokenizer.hasMoreTokens())
                size.y = atof(shapeTokenizer.nextToken());
            if (shapeTokenizer.hasMoreTokens())
                size.z = atof(shapeTokenizer.nextToken());
            shape = new Cuboid(size);
        }
        else if (!strcmp(shapeType, "sphere"))
        {
            double radius = 0;
            if (shapeTokenizer.hasMoreTokens())
                radius = atof(shapeTokenizer.nextToken());
            shape = new Sphere(radius);
        }
        else if (!strcmp(shapeType, "prism"))
        {
            double height;
            if (shapeTokenizer.hasMoreTokens())
                height = atof(shapeTokenizer.nextToken());
            std::vector<Coord> points;
            while (shapeTokenizer.hasMoreTokens())
            {
                Coord point;
                if (shapeTokenizer.hasMoreTokens())
                    point.x = atof(shapeTokenizer.nextToken());
                if (shapeTokenizer.hasMoreTokens())
                    point.y = atof(shapeTokenizer.nextToken());
                points.push_back(point);
            }
            Box boundingBox = Box::calculateBoundingBox(points);
            Coord center = (boundingBox.max - boundingBox.min) / 2 + boundingBox.min;
            center.z = height / 2;
            std::vector<Coord> prismPoints;
            for (std::vector<Coord>::iterator it = points.begin(); it != points.end(); it++)
                prismPoints.push_back(*it - center);
            shape = new Prism(height, Polygon(prismPoints));
        }
        else if (!strcmp(shapeType, "polytope"))
        {
            std::vector<Coord> points;
            while (shapeTokenizer.hasMoreTokens())
            {
                Coord point;
                if (shapeTokenizer.hasMoreTokens())
                    point.x = atof(shapeTokenizer.nextToken());
                if (shapeTokenizer.hasMoreTokens())
                    point.y = atof(shapeTokenizer.nextToken());
                if (shapeTokenizer.hasMoreTokens())
                    point.z = atof(shapeTokenizer.nextToken());
                points.push_back(point);
            }
            Box boundingBox = Box::calculateBoundingBox(points);
            Coord center = (boundingBox.max - boundingBox.min) / 2 + boundingBox.min;
            std::vector<Coord> polytopePoints;
            for (std::vector<Coord>::iterator it = points.begin(); it != points.end(); it++)
                polytopePoints.push_back(*it - center);
            shape = new ConvexPolytope(polytopePoints);
        }
        else {
            int id = atoi(shapeAttribute);
            shape = idToShapeMap[id];
        }
        if (!shape)
            throw cRuntimeError("Unknown shape '%s'", shapeAttribute);
        // position
        Coord position = Coord::NIL;
        const char *positionAttribute = element->getAttribute("position");
        if (positionAttribute)
        {
            cStringTokenizer tokenizer(positionAttribute);
            const char *kind = tokenizer.nextToken();
            if (!kind)
                throw cRuntimeError("Missing position kind");
            else if (!strcmp(kind, "min"))
                position = shape->computeSize() / 2;
            else if (!strcmp(kind, "max"))
                position = shape->computeSize() / -2;
            else if (!strcmp(kind, "center"))
                position = Coord::ZERO;
            else
                throw cRuntimeError("Unknown position kind");
            if (tokenizer.hasMoreTokens())
                position.x += atof(tokenizer.nextToken());
            if (tokenizer.hasMoreTokens())
                position.y += atof(tokenizer.nextToken());
            if (tokenizer.hasMoreTokens())
                position.z += atof(tokenizer.nextToken());
        }
        // material
        const Material *material;
        const char *materialAttribute = element->getAttribute("material");
        if (!materialAttribute)
            throw cRuntimeError("Missing material attribute of object");
        else if (nameToMaterialMap.find(materialAttribute) != nameToMaterialMap.end())
            material = nameToMaterialMap[materialAttribute];
        else if (Material::getMaterial(materialAttribute))
            material = Material::getMaterial(materialAttribute);
        else
            material = idToMaterialMap[atoi(materialAttribute)];
        if (!material)
            throw cRuntimeError("Unknown material '%s'", materialAttribute);
        // line width
        double lineWidth = 1;
        const char *lineWidthAttribute = element->getAttribute("line-width");
        if (lineWidthAttribute)
            lineWidth = atof(lineWidthAttribute);
        // line color
        cFigure::Color lineColor = cFigure::BLACK;
        const char *lineColorAttribute = element->getAttribute("line-color");
        if (lineColorAttribute)
        {
            if (strchr(lineColorAttribute, ' '))
            {
                cStringTokenizer tokenizer(lineColorAttribute);
                if (tokenizer.hasMoreTokens())
                    lineColor.red = atoi(tokenizer.nextToken());
                if (tokenizer.hasMoreTokens())
                    lineColor.green = atoi(tokenizer.nextToken());
                if (tokenizer.hasMoreTokens())
                    lineColor.blue = atoi(tokenizer.nextToken());
            }
            else
                lineColor = cFigure::Color(lineColorAttribute);
        }
        // fill color
        cFigure::Color fillColor = cFigure::WHITE;
        const char *fillColorAttribute = element->getAttribute("fill-color");
        if (fillColorAttribute)
        {
            if (strchr(fillColorAttribute, ' '))
            {
                cStringTokenizer tokenizer(fillColorAttribute);
                if (tokenizer.hasMoreTokens())
                    fillColor.red = atoi(tokenizer.nextToken());
                if (tokenizer.hasMoreTokens())
                    fillColor.green = atoi(tokenizer.nextToken());
                if (tokenizer.hasMoreTokens())
                    fillColor.blue = atoi(tokenizer.nextToken());
            }
            else
                fillColor = cFigure::Color(fillColorAttribute);
        }
        // opacity
        double opacity = 1;
        const char *opacityAttribute = element->getAttribute("opacity");
        if (opacityAttribute)
            opacity = atof(opacityAttribute);
        // tags
        const char *tags = element->getAttribute("tags");
        // insert object
        PhysicalObject *object = new PhysicalObject(name, id, position, orientation, shape, material, lineWidth, lineColor, fillColor, opacity, tags);
        objects.push_back(object);
        if (id != -1)
            idToObjectMap.insert(std::pair<int, const PhysicalObject *>(id, object));
        if (objectCache)
            objectCache->insertObject(object);
    }
    if (objectCache)
        objectCache->buildCache();
}

cFigure::Point PhysicalEnvironment::computeCanvasPoint(const Coord& point, const Rotation& rotation)
{
    Coord rotatedPoint = rotation.rotateVectorClockwise(point);
    return cFigure::Point(rotatedPoint.x, rotatedPoint.y);
}

cFigure::Point PhysicalEnvironment::computeCanvasPoint(Coord point)
{
    // KLUDGE: TODO: don't lookup the environment this way
    PhysicalEnvironment *environment = dynamic_cast<PhysicalEnvironment *>(simulation.getSystemModule()->getSubmodule("environment"));
    if (environment)
        return environment->computeCanvasPoint(point, environment->viewRotation);
    else
        return environment->computeCanvasPoint(point, Rotation(EulerAngles(0,0,0)));
}
void PhysicalEnvironment::updateCanvas()
{
    while (objectsLayer->getNumFigures())
        delete objectsLayer->removeFigure(0);
    // KLUDGE: sorting objects with their rotated position's z coordinate to draw them in a "better" order
    std::vector<const PhysicalObject *> objectsCopy = objects;
    std::sort(objectsCopy.begin(), objectsCopy.end(), ObjectPositionComparator(viewRotation));
    for (std::vector<const PhysicalObject *>::iterator it = objectsCopy.begin(); it != objectsCopy.end(); it++)
    {
        const PhysicalObject *object = *it;
        const Shape3D *shape = object->getShape();
        const Coord& position = object->getPosition();
        const EulerAngles& orientation = object->getOrientation();
        const Rotation rotation(orientation);
        // cuboid
        const Cuboid *cuboid = dynamic_cast<const Cuboid *>(shape);
        if (cuboid)
        {
            std::vector<std::vector<Coord> > faces;
            cuboid->computeVisibleFaces(faces, rotation, viewRotation);
            computeFacePoints(object, faces, rotation);
        }
        // sphere
        const Sphere *sphere = dynamic_cast<const Sphere *>(shape);
        if (sphere)
        {
            double radius = sphere->getRadius();
            cOvalFigure *figure = new cOvalFigure(NULL);
            figure->setFilled(true);
            cFigure::Point topLeft = computeCanvasPoint(position - Coord(radius, radius, radius), viewRotation);
            cFigure::Point bottomRight = computeCanvasPoint(position + Coord(radius, radius, radius), viewRotation);
            figure->setBounds(cFigure::Rectangle(topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y));
            figure->setLineWidth(object->getLineWidth());
            figure->setLineColor(object->getLineColor());
            figure->setFillColor(object->getFillColor());
#if OMNETPP_CANVAS_VERSION>=0x20140908
            figure->setLineOpacity(object->getOpacity());
            figure->setFillOpacity(object->getOpacity());
#endif
            std::string tags("physical_object ");
            if (object->getTags())
                tags += object->getTags();
            figure->setTags(tags.c_str());
            objectsLayer->addFigure(figure);
        }
        // prism
        const Prism *prism = dynamic_cast<const Prism *>(shape);
        if (prism)
        {
            std::vector<std::vector<Coord> > faces;
            prism->computeVisibleFaces(faces, rotation, viewRotation);
            computeFacePoints(object, faces, rotation);
        }
        // polytope
        const ConvexPolytope *polytope = dynamic_cast<const ConvexPolytope *>(shape);
        if (polytope)
        {
            std::vector<std::vector<Coord> > faces;
            polytope->computeVisibleFaces(faces, rotation, viewRotation);
            computeFacePoints(object, faces, rotation);
        }
        // add name to the end
        const char *name = object->getName();
        if (name)
        {
            cLabelFigure *nameFigure = new cLabelFigure(NULL);
            nameFigure->setTags("physical_object object_name label");
            nameFigure->setPosition(computeCanvasPoint(position));
            nameFigure->setText(name);
            objectsLayer->addFigure(nameFigure);
        }
    }
}

void PhysicalEnvironment::computeFacePoints(const PhysicalObject *object, std::vector<std::vector<Coord> >& faces, const Rotation& rotation)
{
    const Coord& position = object->getPosition();
    for (std::vector<std::vector<Coord> >::const_iterator it = faces.begin(); it != faces.end(); it++)
    {
        std::vector<cFigure::Point> canvasPoints;
        const std::vector<Coord>& facePoints = *it;
        for (std::vector<Coord>::const_iterator pit = facePoints.begin(); pit != facePoints.end(); pit++)
        {
            cFigure::Point canvPoint = computeCanvasPoint(rotation.rotateVectorClockwise(*pit) + position, viewRotation);
            canvasPoints.push_back(canvPoint);
        }
        cPolygonFigure *figure = new cPolygonFigure(NULL);
        figure->setFilled(true);
        figure->setPoints(canvasPoints);
        figure->setLineWidth(object->getLineWidth());
        figure->setLineColor(object->getLineColor());
        figure->setFillColor(object->getFillColor());
#if OMNETPP_CANVAS_VERSION>=0x20140908
        figure->setLineOpacity(object->getOpacity());
        figure->setFillOpacity(object->getOpacity());
#endif
        std::string tags("physical_object ");
        if (object->getTags())
            tags += object->getTags();
        figure->setTags(tags.c_str());
        objectsLayer->addFigure(figure);
    }
}

const PhysicalObject *PhysicalEnvironment::getObjectById(int id) const
{
    std::map<int, const PhysicalObject *>::const_iterator it = idToObjectMap.find(id);
    if (it == idToObjectMap.end())
        return NULL;
    else
        return it->second;
}

void PhysicalEnvironment::visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const
{
    if (objectCache)
        objectCache->visitObjects(visitor, lineSegment);
    else
        for (std::vector<const PhysicalObject *>::const_iterator it = objects.begin(); it != objects.end(); it++)
            visitor->visit(*it);
}

void PhysicalEnvironment::handleParameterChange(const char* name)
{
    if (name && !strcmp(name, "viewAngle"))
    {
        viewAngle = computeViewAngle(par("viewAngle"));
        viewRotation = Rotation(viewAngle);
        updateCanvas();
    }
}

EulerAngles PhysicalEnvironment::computeViewAngle(const char* viewAngle)
{
    double x, y, z;
    if (!strcmp(viewAngle, "x"))
    {
        x = 0;
        y = M_PI / 2;
        z = M_PI / -2;
    }
    else if (!strcmp(viewAngle, "y"))
    {
        x = M_PI / 2;
        y = 0;
        z = 0;
    }
    else if (!strcmp(viewAngle, "z"))
    {
        x = y = z = 0;
    }
    else if (sscanf(viewAngle, "%lf %lf %lf", &x, &y, &z) == 3)
    {
        x = math::deg2rad(x);
        y = math::deg2rad(y);
        z = math::deg2rad(z);
    }
    else
        throw cRuntimeError("viewAngle must be a triplet representing three degrees");
    return EulerAngles(x, y, z);
}

} // namespace inet
